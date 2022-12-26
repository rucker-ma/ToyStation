#pragma once

#include "Vulkan/CommandPool.h"
#include "Vulkan/ResourceAllocator.h"

namespace TSEngine {

class Picture {
public:
    Picture()
        : image_(), image_view_(), image_layout_(VK_IMAGE_LAYOUT_UNDEFINED) {}

    Picture(RHIImage image, Texture image_view, VkImageLayout image_layout) {
        image_ = image;
        image_view_ = image_view;
        image_layout_ = image_layout;
    };
    RHIImage image_;
    Texture image_view_;
    VkImageLayout image_layout_;
};
class ReferenceFrameData {
public:
    ReferenceFrameData() {}

    int8_t dpb_idx_;  // -1 when invalid //
    StdVideoEncodeH264RefPicMarkingEntry std_ref_pic_data_;
    int32_t poc_;  // index in video sequence - picture order count
};
class EncodeFrameData {
public:
    Picture picture_;
    ReferenceFrameData ref_pics_[16];
    uint32_t used_dbp_mask_;
    int32_t ref_count_;
    VkDevice device_;
    VkFence frame_complete_fence_;
    VkSemaphore frame_encoded_semaphore_;
    VkFence frame_consumer_done_fence_;
    VkSemaphore frame_producer_done_semaphore_;
    uint32_t queue_family_index_;
    VkVideoProfileInfoKHR video_profile_;
    VkExtent2D extent_;
    Buffer out_bistream_buffer_;
    Buffer input_staging_buffer_;
    VkCommandBuffer encode_command_buffer_;
    uint32_t frame_submitted : 1;
};

class PictureBuffer {
public:
    PictureBuffer() = default;
    VkResult CreateVideoQueries(uint32_t num_slots,
                                const VkVideoProfileInfoKHR* encode_profile);
    int32_t InitFramePool(VkDevice device, VkVideoProfileInfoKHR* encode_profile,
                          uint32_t num_images, VkFormat image_format,
                          uint32_t max_image_width, uint32_t max_image_height,
                          uint32_t full_image_size, VkImageTiling tiling,
                          VkImageUsageFlags usage,
                          DedicatedResourceAllocator* alloc,
                          CommandPool* encode_pool,
                          uint32_t queue_family_index);
    void DeInitFramePool();
    void PrepareInputImages(VkCommandBuffer cmd_buffer);
    void InitReferenceFramePool(uint32_t num_images, VkFormat image_format,
                                VkResourceAllocator* alloc);
    void PrepareReferenceImages(VkCommandBuffer cmd_buffer);
    void GetFrameResourcesByIndex(
        int8_t reference_slot_index,
        VkVideoPictureResourceInfoKHR* picture_resources);
    void GetReferenceFrameResourcesByIndex(
        int8_t dpb_slot_idx, VkVideoPictureResourceInfoKHR* picture_resources);
    uint32_t InitFrame(uint32_t num_images, VkDevice dev,
                       const VkImageCreateInfo* image_create_info,
                       DedicatedResourceAllocator* alloc,
                       CommandPool* encode_pool,
                       VkMemoryPropertyFlags reqs_mem_props = 0,
                       int32_t init_with_pattern = -1,
                       VkExternalMemoryHandleTypeFlagBitsKHR export_handle =
                           VkExternalMemoryHandleTypeFlagBitsKHR());
    void AddRefPicture(uint8_t image_idx, int8_t dpb_index, uint32_t poc);
    void RemoveRefPicture(uint8_t image_idx);
    int32_t ConfigRefPicture(uint8_t dist_anchors, uint8_t distance_intras,
                             uint32_t current_poc,
                             uint8_t current_encode_frame_idx);
    VkResult CopyToVkImage(uint32_t index, uint32_t buffer_offset,
                           VkCommandBuffer cmd_buffer);
    EncodeFrameData* GetEncodeFrameData(uint32_t index);
    VkQueryPool GetQueryPool() { return query_pool_; }

private:
    static void InitImageLayout(VkCommandBuffer cmd_buffer, Picture* picture,
                                VkImageLayout layout);

    VkDevice device_;
    uint32_t queue_family_index_;
    VkVideoProfileInfoKHR* video_profile_;
    VkImageCreateInfo image_create_info_;
    size_t framebuffer_size_;
    size_t dpb_size_;
    uint32_t max_bitstream_size_;
    std::vector<EncodeFrameData> encode_frame_data_;
    DedicatedResourceAllocator* resource_alloc_;
    std::vector<Picture> dpb_;
    VkQueryPool query_pool_;
    VkExtent2D extent_;
    uint32_t full_image_size_;
    VkFormat image_format_;
};
}  // namespace TSEngine