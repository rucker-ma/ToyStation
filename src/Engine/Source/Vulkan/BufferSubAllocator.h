#pragma once
#include "MemoryAllocator.h"
#include "TrangleAllocator.h"
#include "VkContext.h"

namespace toystation {

class BufferSubAllocator {
private:
    static const uint32_t INVALID_ID_INDEX = ~0;
    static const uint32_t BASE_ALIGNMENT =
        16;  // could compromise between max block size and typical requests

public:
    class Handle {
        friend class BufferSubAllocator;

    private:
        static const uint32_t BLOCKBITS = 26;

        // if we cannot pack size and offset each into 26 bits (after adjusting
        // for base alignment) we need a dedicated block just for this
        static bool NeedsDedicated(uint64_t size, uint64_t alignment) {
            return ((size + (alignment > 16 ? alignment : 0)) >=
                    (uint64_t((1 << BLOCKBITS)) * uint64_t(BASE_ALIGNMENT)));
        }

        union {
            struct {
                uint64_t block_index : 11;  // 2047 blocks, typical blockSize 64
                                            // MB or more, should be enough
                uint64_t offset : BLOCKBITS;
                uint64_t size : BLOCKBITS;
                uint64_t dedicated : 1;  // 0 dedicated or not
            };
            uint64_t raw;
        };

        uint64_t GetOffset() const {
            return dedicated == 1 ? 0 : offset * uint64_t(BASE_ALIGNMENT);
        }
        uint64_t GetSize() const {
            return dedicated == 1 ? offset + (size << BLOCKBITS)
                                  : size * uint64_t(BASE_ALIGNMENT);
        }
        uint32_t GetBlockIndex() const { return uint32_t(block_index); }
        bool IsDedicated() const { return dedicated == 1; }

        bool Setup(uint32_t blockIndex_, uint64_t offset_, uint64_t size_,
                   bool dedicated_) {
            const uint64_t blockBitsMask = ((1ULL << BLOCKBITS) - 1);
            assert((blockIndex_ & ~((1ULL << 11) - 1)) == 0);
            block_index = blockIndex_ & ((1ULL << 11) - 1);
            if (dedicated_) {
                dedicated = 1;
                offset = size_ & blockBitsMask;
                size = (size_ >> BLOCKBITS) & blockBitsMask;
            } else {
                dedicated = 0;
                offset = (offset_ / uint64_t(BASE_ALIGNMENT)) & blockBitsMask;
                size = (size_ / uint64_t(BASE_ALIGNMENT)) & blockBitsMask;
            }

            return (GetBlockIndex() == blockIndex_ && GetOffset() == offset_ &&
                    GetSize() == size_);
        }

    public:
        Handle() { raw = ~uint64_t(0); }

        bool IsValid() const { return raw != ~uint64_t(0); }
        bool IsEqual(const Handle& other) const {
            return block_index == other.block_index && offset == other.offset &&
                   dedicated == other.dedicated && size == other.size;
        }

        operator bool() const { return IsValid(); }

        friend bool operator==(const Handle& lhs, const Handle& rhs) {
            return rhs.IsEqual(lhs);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    BufferSubAllocator(BufferSubAllocator const&) = delete;
    BufferSubAllocator& operator=(BufferSubAllocator const&) = delete;
    BufferSubAllocator() = default;
    BufferSubAllocator(
        VkMemoryAllocator* allocator, VkDeviceSize block_size,
        VkBufferUsageFlags buffer_usage_flags,
        VkMemoryPropertyFlags mem_prop_flags =
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        bool mapped = false,
        const std::vector<uint32_t>& sharing_queue_family_indices =
            std::vector<uint32_t>()) {
        Init(allocator, block_size, buffer_usage_flags, mem_prop_flags, mapped,
             sharing_queue_family_indices);
    }

    ~BufferSubAllocator() { DeInit(); }

    void Init(
        VkMemoryAllocator* allocator, VkDeviceSize block_size,
        VkBufferUsageFlags buffer_usage_flags,
        VkMemoryPropertyFlags mem_prop_flags =
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        bool mapped = false,
        const std::vector<uint32_t>& sharing_queues = std::vector<uint32_t>());
    void DeInit();

    void SetKeepLastBlockOnFree(bool state) { keep_last_block_ = state; }

    // alignment will be BASE_ALIGNMENT byte at least
    // alignment must be power of 2
    Handle SubAllocate(VkDeviceSize size, uint32_t alignment = BASE_ALIGNMENT);
    void SubFree(Handle sub);

    struct Binding {
        VkBuffer buffer;
        uint64_t offset;
        uint64_t size;
        VkDeviceAddress address;
    };

    // sub allocation was aligned to BASE_ALIGNMENT
    Binding GetSubBinding(Handle handle) {
        Binding binding;
        binding.offset = handle.GetOffset();
        binding.size = handle.GetSize();
        binding.buffer = blocks_[handle.GetBlockIndex()].buffer;
        binding.address =
            blocks_[handle.GetBlockIndex()].address + binding.offset;

        return binding;
    }
    // sub allocation alignment was custom
    Binding GetSubBinding(Handle handle, uint32_t alignment) {
        Binding binding;
        binding.offset = (handle.GetOffset() + (uint64_t(alignment) - 1)) &
                         ~(uint64_t(alignment) - 1);
        binding.size = handle.GetSize() - (binding.offset - handle.GetOffset());
        binding.buffer = blocks_[handle.GetBlockIndex()].buffer;
        binding.address =
            blocks_[handle.GetBlockIndex()].address + binding.offset;

        return binding;
    }

    void* GetSubMapping(Handle handle,
                        uint32_t alignment = BASE_ALIGNMENT) const {
        return blocks_[handle.GetBlockIndex()].mapping +
               ((handle.GetOffset() + (uint64_t(alignment) - 1)) &
                ~(uint64_t(alignment) - 1));
    }

    uint32_t GetSubBlockIndex(Handle handle) const {
        return handle.GetBlockIndex();
    }
    VkBuffer GetBlockBuffer(uint32_t blockIndex) const {
        return blocks_[blockIndex].buffer;
    }

    float GetUtilization(VkDeviceSize& allocatedSize,
                         VkDeviceSize& usedSize) const;
    bool FitsInAllocated(VkDeviceSize size,
                         uint32_t alignment = BASE_ALIGNMENT) const;

    void Free(bool onlyEmpty);

protected:
    // - Block stores VkBuffers that we sub-allocate the staging space from

    // To recycle Block structures within the arrays
    // we use a linked list of array indices. The "index" element
    // in the struct refers to the next free list item, or itself
    // when in use.
    // A block is "dedicated" if it only holds a single allocation.
    // This can happen if we cannot encode the offset/size into the
    // bits that the Handle provides for this, or when the size
    // of the allocation is bigger than our preferred block size.

    struct Block {
        uint32_t index = INVALID_ID_INDEX;
        VkDeviceSize size = 0;
        VkBuffer buffer = VK_NULL_HANDLE;
        TRangeAllocator<BASE_ALIGNMENT> range;
        MemHandle memory = nullptr;
        uint8_t* mapping = nullptr;
        VkDeviceAddress address = 0;
        bool is_dedicated = false;
    };

    VkMemoryAllocator* allocator_ = nullptr;
    VkDevice device_ = VK_NULL_HANDLE;
    uint32_t memory_type_index_;
    VkDeviceSize block_size_;
    VkBufferUsageFlags buffer_usage_flags_;
    VkMemoryPropertyFlags memory_prop_flags_;
    std::vector<uint32_t> sharing_queue_family_indices_;
    bool mapped_;
    bool keep_last_block_ = false;

    std::vector<Block> blocks_;
    uint32_t regular_blocks_ = 0;
    uint32_t free_block_index_;  // linked list to next free block
    VkDeviceSize allocated_size_;
    VkDeviceSize used_size_;

    uint32_t SetIndexValue(uint32_t& index, uint32_t new_value) {
        uint32_t old_value = index;
        index = new_value;
        return old_value;
    }

    Block& GetBlock(uint32_t index) {
        Block& block = blocks_[index];
        assert(block.index == index);
        return block;
    }

    void FreeBlock(Block& block);
    VkResult AllocBlock(Block& block, uint32_t id, VkDeviceSize size);
};

}  // namespace toystation