#include "BufferSubAllocator.h"
namespace toystation {

void BufferSubAllocator::Init(VkMemoryAllocator* allocator,
                              VkDeviceSize block_size,
                              VkBufferUsageFlags buffer_usage_flags,
                              VkMemoryPropertyFlags mem_prop_flags, bool mapped,
                              const std::vector<uint32_t>& sharing_queues) {
    assert(!device_);
    allocator_ = allocator;
    device_ = allocator->GetDevice();
    block_size_ =
        std::min(block_size, ((uint64_t(1) << Handle::BLOCKBITS) - 1) *
                                 uint64_t(BASE_ALIGNMENT));
    buffer_usage_flags_ = buffer_usage_flags;
    memory_prop_flags_ = mem_prop_flags;
    memory_type_index_ = ~0;
    keep_last_block_ = true;
    mapped_ = mapped;
    sharing_queue_family_indices_ = sharing_queues;

    free_block_index_ = INVALID_ID_INDEX;
    used_size_ = 0;
    allocated_size_ = 0;
}

void BufferSubAllocator::DeInit() {
    if (!allocator_) return;

    Free(false);

    blocks_.clear();
    allocator_ = nullptr;
}

BufferSubAllocator::Handle BufferSubAllocator::SubAllocate(VkDeviceSize size,
                                                           uint32_t align) {
    uint32_t used_offset;
    uint32_t used_size;
    uint32_t used_aligned;

    uint32_t block_index = INVALID_ID_INDEX;

    // if size either doesn't fit in the bits within the handle
    // or we are bigger than the default block size, we use a full dedicated
    // block for this allocation
    bool is_dedicated =
        Handle::NeedsDedicated(size, align) || size > block_size_;

    if (!is_dedicated) {
        // Find the first non-dedicated block that can fit the allocation
        for (uint32_t i = 0; i < (uint32_t)blocks_.size(); i++) {
            Block& block = blocks_[i];
            if (!block.is_dedicated && block.buffer &&
                block.range.SubAllocate((uint32_t)size, align, used_offset,
                                        used_aligned, used_size)) {
                block_index = block.index;
                break;
            }
        }
    }

    if (block_index == INVALID_ID_INDEX) {
        if (free_block_index_ != INVALID_ID_INDEX) {
            Block& block = blocks_[free_block_index_];
            free_block_index_ = SetIndexValue(block.index, free_block_index_);

            block_index = block.index;
        } else {
            uint32_t new_index = (uint32_t)blocks_.size();
            blocks_.resize(blocks_.size() + 1);
            Block& block = blocks_[new_index];
            block.index = new_index;

            block_index = new_index;
        }

        Block& block = blocks_[block_index];
        block.size = std::max(block_size_, size);
        if (!is_dedicated) {
            // only adjust size if not dedicated.
            // warning this lowers from 64 bit to 32 bit size, which should be
            // fine given such big allocations will trigger the dedicated path
            block.size = block.range.AlignedSize((uint32_t)block.size);
        }

        VkResult result = AllocBlock(block, block_index, block.size);

        if (result != VK_SUCCESS) {
            FreeBlock(block);
            return Handle();
        }

        block.is_dedicated = is_dedicated;

        if (!is_dedicated) {
            // Dedicated blocks don't allow for subranges, so don't initialize
            // the range allocator
            block.range.Init((uint32_t)block.size);
            block.range.SubAllocate((uint32_t)size, align, used_offset,
                                    used_aligned, used_size);
            regular_blocks_++;
        }
    }

    Handle sub;
    if (!sub.Setup(block_index, is_dedicated ? 0 : used_offset,
                   is_dedicated ? size : uint64_t(used_size), is_dedicated)) {
        return Handle();
    }

    // append used space for stats
    used_size_ += sub.GetSize();

    return sub;
}

void BufferSubAllocator::SubFree(Handle sub) {
    if (!sub) return;

    Block& block = GetBlock(sub.block_index);
    bool is_dedicated = sub.IsDedicated();
    if (!is_dedicated) {
        block.range.SubFree(uint32_t(sub.GetOffset()), uint32_t(sub.GetSize()));
    }

    used_size_ -= sub.GetSize();

    if (is_dedicated ||
        (block.range.IsEmpty() && (!keep_last_block_ || regular_blocks_ > 1))) {
        if (!is_dedicated) {
            regular_blocks_--;
        }
        FreeBlock(block);
    }
}

float BufferSubAllocator::GetUtilization(VkDeviceSize& allocated_size,
                                         VkDeviceSize& used_size) const {
    allocated_size = allocated_size_;
    used_size = used_size_;

    return float(double(used_size) / double(allocated_size));
}

bool BufferSubAllocator::FitsInAllocated(VkDeviceSize size,
                                         uint32_t alignment) const {
    if (Handle::NeedsDedicated(size, alignment)) {
        return false;
    }

    for (const auto& block : blocks_) {
        if (block.buffer && !block.is_dedicated) {
            if (block.range.IsAvailable((uint32_t)size, (uint32_t)alignment)) {
                return true;
            }
        }
    }

    return false;
}

void BufferSubAllocator::Free(bool only_empty) {
    for (uint32_t i = 0; i < (uint32_t)blocks_.size(); i++) {
        Block& block = blocks_[i];
        if (block.buffer &&
            (!only_empty || (!block.is_dedicated && block.range.IsEmpty()))) {
            FreeBlock(block);
        }
    }

    if (!only_empty) {
        blocks_.clear();
        free_block_index_ = INVALID_ID_INDEX;
    }
}

void BufferSubAllocator::FreeBlock(Block& block) {
    allocated_size_ -= block.size;

    vkDestroyBuffer(device_, block.buffer, nullptr);
    if (block.mapping) {
        allocator_->UnMap(block.memory);
    }
    allocator_->FreeMemory(block.memory);

    if (!block.is_dedicated) {
        block.range.DeInit();
    }
    block.memory = nullptr;
    block.buffer = VK_NULL_HANDLE;
    block.mapping = nullptr;
    block.is_dedicated = false;

    // update the block.index with the current head of the free list
    // pop its old value
    free_block_index_ = SetIndexValue(block.index, free_block_index_);
}

VkResult BufferSubAllocator::AllocBlock(Block& block, uint32_t index,
                                        VkDeviceSize size) {
    VkResult result = VK_SUCCESS;
    VkBufferCreateInfo create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    create_info.size = size;
    create_info.usage = buffer_usage_flags_;
    create_info.sharingMode = sharing_queue_family_indices_.size() > 1
                                  ? VK_SHARING_MODE_CONCURRENT
                                  : VK_SHARING_MODE_EXCLUSIVE;
    create_info.pQueueFamilyIndices = sharing_queue_family_indices_.data();
    create_info.queueFamilyIndexCount =
        static_cast<uint32_t>(sharing_queue_family_indices_.size());

    VkBuffer buffer = VK_NULL_HANDLE;
    result = vkCreateBuffer(device_, &create_info, nullptr, &buffer);
    if (result != VK_SUCCESS) {
        return result;
    }

    VkMemoryRequirements2 mem_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2};
    VkBufferMemoryRequirementsInfo2 buffer_reqs = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2};

    buffer_reqs.buffer = buffer;
    vkGetBufferMemoryRequirements2(device_, &buffer_reqs, &mem_reqs);

    if (memory_type_index_ == ~0) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(allocator_->GetPhysicalDevice(),
                                            &memory_properties);

        VkMemoryPropertyFlags mem_props = memory_prop_flags_;

        // Find an available memory type that satisfies the requested
        // properties.
        for (uint32_t memory_type_index = 0;
             memory_type_index < memory_properties.memoryTypeCount;
             ++memory_type_index) {
            if ((mem_reqs.memoryRequirements.memoryTypeBits &
                 (1 << memory_type_index)) &&
                (memory_properties.memoryTypes[memory_type_index]
                     .propertyFlags &
                 mem_props) == mem_props) {
                memory_type_index_ = memory_type_index;
                break;
            }
        }
    }

    if (memory_type_index_ == ~0) {
        assert(0 && "could not find memory_type_index\n");
        vkDestroyBuffer(device_, buffer, nullptr);
        return VK_ERROR_INCOMPATIBLE_DRIVER;
    }

    MemoryAllocateInfo mem_allocate_info(mem_reqs.memoryRequirements,
                                       memory_prop_flags_, false);

    MemHandle memory = allocator_->AllocMemory(mem_allocate_info);
    if (result != VK_SUCCESS) {
        assert(0 && "could not allocate buffer\n");
        vkDestroyBuffer(device_, buffer, nullptr);
        return result;
    }

    VkMemoryAllocator::MemoryInfo mem_info = allocator_->GetMemoryInfo(memory);

    VkBindBufferMemoryInfo bind_infos = {
        VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO};
    bind_infos.buffer = buffer;
    bind_infos.memory = mem_info.memory;
    bind_infos.memoryOffset = mem_info.offset;

    result = vkBindBufferMemory2(device_, 1, &bind_infos);

    if (result == VK_SUCCESS) {
        if (mapped_) {
            block.mapping = (uint8_t*)allocator_->Map(memory);
        } else {
            block.mapping = nullptr;
        }

        if (!mapped_ || block.mapping) {
            if (buffer_usage_flags_ &
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                VkBufferDeviceAddressInfo info = {
                    VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
                info.buffer = buffer;
                block.address = vkGetBufferDeviceAddress(device_, &info);
            }

            block.memory = memory;
            block.buffer = buffer;
            allocated_size_ += block.size;
            return result;
        }
    }

    // error case

    vkDestroyBuffer(device_, buffer, nullptr);
    allocator_->FreeMemory(memory);
    return result;
}
}  // namespace toystation