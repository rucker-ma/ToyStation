#include "StagingMemoryManager.h"
namespace toystation {
StagingMemoryManager::StagingMemoryManager(VkMemoryAllocator* allocator,
                                           VkDeviceSize staging_block_size)
    : staging_index_(0), free_staging_index_(0) {
    Init(allocator, staging_block_size);
}

void StagingMemoryManager::Init(
    VkMemoryAllocator* allocator,
    VkDeviceSize staging_block_size /*= 64 * 1024 * 1024*/) {
    assert(!device_);
    device_ = allocator->GetDevice();

    sub_to_device_.Init(allocator, staging_block_size,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        true);
    sub_from_device_.Init(allocator, staging_block_size,
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                              VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
                          true);

    free_staging_index_ = INVALID_ID_INDEX;
    staging_index_ = NewStagingIndex();

    SetFreeUnusedOnRelease(true);
}

void StagingMemoryManager::DeInit() {
    if (!device_) return;

    Free(false);

    sub_from_device_.DeInit();
    sub_to_device_.DeInit();

    sets_.clear();
    device_ = VK_NULL_HANDLE;
}

bool StagingMemoryManager::FitsInAllocated(VkDeviceSize size,
                                           bool to_device /*= true*/) const {
    return to_device ? sub_to_device_.FitsInAllocated(size)
                     : sub_from_device_.FitsInAllocated(size);
}

void* StagingMemoryManager::CmdToImage(
    VkCommandBuffer cmd, VkImage image, const VkOffset3D& offset,
    const VkExtent3D& extent, const VkImageSubresourceLayers& subresource,
    VkDeviceSize size, const void* data, VkImageLayout layout) {
    if (!image) {
        return nullptr;
    }

    VkBuffer src_buffer = nullptr;
    VkDeviceSize src_offset = 0;

    void* mapping = GetStagingSpace(size, src_buffer, src_offset, true);

    assert(mapping);

    if (data) {
        memcpy(mapping, data, size);
    }

    VkBufferImageCopy cpy;
    cpy.bufferOffset = src_offset;
    cpy.bufferRowLength = 0;
    cpy.bufferImageHeight = 0;
    cpy.imageSubresource = subresource;
    cpy.imageOffset = offset;
    cpy.imageExtent = extent;

    vkCmdCopyBufferToImage(cmd, src_buffer, image, layout, 1, &cpy);

    return data ? nullptr : mapping;
}

void* StagingMemoryManager::CmdToBuffer(VkCommandBuffer cmd, VkBuffer buffer,
                                        VkDeviceSize offset, VkDeviceSize size,
                                        const void* data) {
    if (!size || !buffer) {
        return nullptr;
    }

    VkBuffer src_buffer = nullptr;
    VkDeviceSize src_offset = 0;

    void* mapping = GetStagingSpace(size, src_buffer, src_offset, true);

    assert(mapping);

    if (data) {
        memcpy(mapping, data, size);
    }

    VkBufferCopy cpy;
    cpy.size = size;
    cpy.srcOffset = src_offset;
    cpy.dstOffset = offset;

    vkCmdCopyBuffer(cmd, src_buffer, buffer, 1, &cpy);

    return data ? nullptr : (void*)mapping;
}

const void* StagingMemoryManager::CmdFromBuffer(VkCommandBuffer cmd,
                                                VkBuffer buffer,
                                                VkDeviceSize offset,
                                                VkDeviceSize size) {
    VkBuffer dst_buffer = nullptr;
    VkDeviceSize dst_offset = 0;
    void* mapping = GetStagingSpace(size, dst_buffer, dst_offset, false);

    VkBufferCopy cpy;
    cpy.size = size;
    cpy.srcOffset = offset;
    cpy.dstOffset = dst_offset;

    vkCmdCopyBuffer(cmd, buffer, dst_buffer, 1, &cpy);

    return mapping;
}

const void* StagingMemoryManager::CmdFromImage(
    VkCommandBuffer cmd, VkImage image, const VkOffset3D& offset,
    const VkExtent3D& extent, const VkImageSubresourceLayers& subresource,
    VkDeviceSize size, VkImageLayout layout) {
    VkBuffer dst_buffer = nullptr;
    VkDeviceSize dst_offset = 0;
    void* mapping = GetStagingSpace(size, dst_buffer, dst_offset, false);

    VkBufferImageCopy cpy;
    cpy.bufferOffset = dst_offset;
    cpy.bufferRowLength = 0;
    cpy.bufferImageHeight = 0;
    cpy.imageSubresource = subresource;
    cpy.imageOffset = offset;
    cpy.imageExtent = extent;

    vkCmdCopyImageToBuffer(cmd, image, layout, dst_buffer, 1, &cpy);

    return mapping;
}

void StagingMemoryManager::FinalizeResources(VkFence fence) {
    if (sets_[staging_index_].entries.empty()) {
        return;
    }

    sets_[staging_index_].fence = fence;
    sets_[staging_index_].manualSet = false;
    staging_index_ = NewStagingIndex();
}

StagingMemoryManager::SetID StagingMemoryManager::FinalizeResourceSet() {
    SetID set_id;

    if (sets_[staging_index_].entries.empty()) {
        return set_id;
    }

    set_id.index_ = staging_index_;

    sets_[staging_index_].fence = nullptr;
    sets_[staging_index_].manualSet = true;
    staging_index_ = NewStagingIndex();

    return set_id;
}

void* StagingMemoryManager::GetStagingSpace(VkDeviceSize size, VkBuffer& buffer,
                                            VkDeviceSize& offset,
                                            bool to_device) {
    assert(sets_[staging_index_].index == staging_index_ &&
           "illegal index, did you forget finalizeResources");

    BufferSubAllocator::Handle handle =
        to_device ? sub_to_device_.SubAllocate(size)
                  : sub_from_device_.SubAllocate(size);
    assert(handle);

    BufferSubAllocator::Binding info =
        to_device ? sub_to_device_.GetSubBinding(handle)
                  : sub_from_device_.GetSubBinding(handle);
    buffer = info.buffer;
    offset = info.offset;

    // append used space to current staging set list
    sets_[staging_index_].entries.push_back({handle, to_device});

    return to_device ? sub_to_device_.GetSubMapping(handle)
                     : sub_from_device_.GetSubMapping(handle);
}

void StagingMemoryManager::ReleaseResources(uint32_t staging_id) {
    if (staging_id == INVALID_ID_INDEX) {
        return;
    }

    StagingSet& set = sets_[staging_id];
    assert(set.index == staging_id);

    // free used allocation ranges
    for (auto& itentry : set.entries) {
        if (itentry.toDevice) {
            sub_to_device_.SubFree(itentry.handle);
        } else {
            sub_from_device_.SubFree(itentry.handle);
        }
    }
    set.entries.clear();

    // update the set.index with the current head of the free list
    // pop its old value
    free_staging_index_ = SetIndexValue(set.index, free_staging_index_);
}

void StagingMemoryManager::ReleaseResources() {
    for (auto& itset : sets_) {
        if (!itset.entries.empty() && !itset.manualSet &&
            (!itset.fence ||
             vkGetFenceStatus(device_, itset.fence) == VK_SUCCESS)) {
            ReleaseResources(itset.index);
            itset.fence = NULL;
            itset.manualSet = false;
        }
    }
    // special case for ease of use if there is only one
    if (staging_index_ == 0 && free_staging_index_ == 0) {
        free_staging_index_ = SetIndexValue(sets_[0].index, 0);
    }
}

float StagingMemoryManager::GetUtilization(VkDeviceSize& allocated_size,
                                           VkDeviceSize& used_size) const {
    VkDeviceSize asize = 0;
    VkDeviceSize usize = 0;
    sub_from_device_.GetUtilization(asize, usize);

    allocated_size = asize;
    used_size = usize;
    sub_to_device_.GetUtilization(asize, usize);
    allocated_size += asize;
    used_size += usize;

    return float(double(used_size) / double(allocated_size));
}

void StagingMemoryManager::Free(bool unused_only) {
    sub_to_device_.Free(unused_only);
    sub_from_device_.Free(unused_only);
}

uint32_t StagingMemoryManager::NewStagingIndex() {
    // find free slot
    if (free_staging_index_ != INVALID_ID_INDEX) {
        uint32_t new_index = free_staging_index_;
        // this updates the free link-list
        free_staging_index_ = SetIndexValue(sets_[new_index].index, new_index);
        assert(sets_[new_index].index == new_index);
        return sets_[new_index].index;
    }

    // otherwise push to end
    uint32_t new_index = (uint32_t)sets_.size();

    StagingSet info;
    info.index = new_index;
    sets_.push_back(info);

    assert(sets_[new_index].index == new_index);
    return new_index;
}
}  // namespace toystation