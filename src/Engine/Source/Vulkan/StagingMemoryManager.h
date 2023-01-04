#pragma once
#include "VkContext.h"
#include "BufferSubAllocator.h"


namespace toystation {

class StagingMemoryManager {
public:
    static const uint32_t INVALID_ID_INDEX = ~0;

    //////////////////////////////////////////////////////////////////////////
    class SetID {
        friend StagingMemoryManager;

    private:
        uint32_t index_ = INVALID_ID_INDEX;
    };

    StagingMemoryManager(StagingMemoryManager const&) = delete;
    StagingMemoryManager& operator=(StagingMemoryManager const&) = delete;

    StagingMemoryManager(
        VkMemoryAllocator* allocator,
        VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);

    virtual ~StagingMemoryManager() { DeInit(); }

    void Init(VkMemoryAllocator* allocator,
              VkDeviceSize taging_block_size = DEFAULT_STAGING_BLOCKSIZE);
    void DeInit();

    // if true (default) we free the memory completely when released
    // otherwise we would keep blocks for re-use around, unless freeUnused() is
    // called
    void SetFreeUnusedOnRelease(bool state) {
        sub_to_device_.SetKeepLastBlockOnFree(!state);
        sub_from_device_.SetKeepLastBlockOnFree(!state);
    }

    // test if there is enough space in current allocations
    bool FitsInAllocated(VkDeviceSize size, bool to_device = true) const;

    // if data != nullptr memcpies to mapping and returns nullptr
    // otherwise returns temporary mapping (valid until "complete" functions)
    void* CmdToImage(
        VkCommandBuffer cmd, VkImage image, const VkOffset3D& offset,
        const VkExtent3D& extent, const VkImageSubresourceLayers& subresource,
        VkDeviceSize size, const void* data,
        VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    template <class T>
    T* CmdToImageT(
        VkCommandBuffer cmd, VkImage image, const VkOffset3D& offset,
        const VkExtent3D& extent, const VkImageSubresourceLayers& subresource,
        VkDeviceSize size, const void* data,
        VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        return (T*)cmdToImage(cmd, image, offset, extent, subresource, size,
                              data, layout);
    }

    // pointer can be used after cmd execution but only valid until associated
    // resources haven't been released
    const void* CmdFromImage(
        VkCommandBuffer cmd, VkImage image, const VkOffset3D& offset,
        const VkExtent3D& extent, const VkImageSubresourceLayers& subresource,
        VkDeviceSize size,
        VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    template <class T>
    const T* CmdFromImageT(
        VkCommandBuffer cmd, VkImage image, const VkOffset3D& offset,
        const VkExtent3D& extent, const VkImageSubresourceLayers& subresource,
        VkDeviceSize size,
        VkImageLayout layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        return (const T*)CmdFromImage(cmd, image, offset, extent, subresource,
                                      size, layout);
    }

    // if data != nullptr memcpies to mapping and returns nullptr
    // otherwise returns temporary mapping (valid until appropriate release)
    void* CmdToBuffer(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset,
                      VkDeviceSize size, const void* data);

    template <class T>
    T* CmdToBufferT(VkCommandBuffer cmd, VkBuffer buffer, VkDeviceSize offset,
                    VkDeviceSize size) {
        return (T*)CmdToBuffer(cmd, buffer, offset, size, nullptr);
    }

    // pointer can be used after cmd execution but only valid until associated
    // resources haven't been released
    const void* CmdFromBuffer(VkCommandBuffer cmd, VkBuffer buffer,
                              VkDeviceSize offset, VkDeviceSize size);

    template <class T>
    const T* CmdFromBufferT(VkCommandBuffer cmd, VkBuffer buffer,
                            VkDeviceSize offset, VkDeviceSize size) {
        return (const T*)CmdFromBuffer(cmd, buffer, offset, size);
    }

    // closes the batch of staging resources since last finalize call
    // and associates it with a fence for later release.
    void FinalizeResources(VkFence fence = VK_NULL_HANDLE);

    // releases the staging resources whose fences have completed
    // and those who had no fence at all, skips resourceSets.
    void ReleaseResources();

    // closes the batch of staging resources since last finalize call
    // and returns a resource set handle that can be used to release them
    SetID FinalizeResourceSet();

    // releases the staging resources from this particular
    // resource set.
    void ReleaseResourceSet(SetID setid) { ReleaseResources(setid.index_); }

    // frees staging memory no longer in use
    void FreeUnused() { Free(true); }

    float GetUtilization(VkDeviceSize& allocated_size,
                         VkDeviceSize& used_size) const;

protected:
    // The implementation uses two major arrays:
    // - Block stores VkBuffers that we sub-allocate the staging space from
    // - StagingSet stores all such sub-allocations that were used
    //   in one batch of operations. Each batch is closed with
    //   finalizeResources, and typically associated with a fence.
    //   As such the resources are given by for recycling if the fence
    //   completed.

    // To recycle StagingSet structures within the arrays
    // we use a linked list of array indices. The "index" element
    // in the struct refers to the next free list item, or itself
    // when in use.

    struct Entry {
        BufferSubAllocator::Handle handle;
        bool toDevice;
    };

    struct StagingSet {
        uint32_t index = INVALID_ID_INDEX;
        VkFence fence = VK_NULL_HANDLE;
        bool manualSet = false;
        std::vector<Entry> entries;
    };

    VkDevice device_ = VK_NULL_HANDLE;

    BufferSubAllocator sub_to_device_;
    BufferSubAllocator sub_from_device_;

    std::vector<StagingSet> sets_;

    // active staging Index, must be valid at all items
    uint32_t staging_index_;
    // linked-list to next free staging set
    uint32_t free_staging_index_;

    uint32_t SetIndexValue(uint32_t& index, uint32_t newValue) {
        uint32_t oldValue = index;
        index = newValue;
        return oldValue;
    }

    void Free(bool unused_only);

    uint32_t NewStagingIndex();

    void* GetStagingSpace(VkDeviceSize size, VkBuffer& buffer,
                          VkDeviceSize& offset, bool to_device);

    void ReleaseResources(uint32_t staging_id);
};
}  // namespace toystation