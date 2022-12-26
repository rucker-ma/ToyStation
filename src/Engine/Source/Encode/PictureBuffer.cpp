#include "PictureBuffer.h"

#include "Vulkan/VkImageUtil.h"

namespace TSEngine {

inline VkBufferCreateInfo MakeBufferCreateInfo(VkDeviceSize size,
                                               VkBufferUsageFlags usage,
                                               VkBufferCreateFlags flags = 0) {
    VkBufferCreateInfo create_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    create_info.size = size;
    create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    create_info.flags = flags;

    return create_info;
}
VkImageViewCreateInfo MakeImage2DViewCreateInfo(
    VkImage image, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
    VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT,
    uint32_t levels = 1, const void* next_image_view = nullptr) {
    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.pNext = next_image_view;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = levels;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    return view_info;
}
VkResult PictureBuffer::CreateVideoQueries(
    uint32_t num_slots, const VkVideoProfileInfoKHR* encode_profile) {
    VkQueryPoolCreateInfo create_info;
    ZeroVKStruct(create_info, VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO);
    create_info.queryType =
        VK_QUERY_TYPE_VIDEO_ENCODE_BITSTREAM_BUFFER_RANGE_KHR;
    create_info.queryCount = num_slots * 2;
    create_info.pNext = encode_profile;

    return vkCreateQueryPool(device_, &create_info, nullptr, &query_pool_);
}
int32_t PictureBuffer::InitFramePool(
    VkDevice device, VkVideoProfileInfoKHR* encode_profile, uint32_t num_images,
    VkFormat image_format, uint32_t max_image_width, uint32_t max_image_height,
    uint32_t full_image_size, VkImageTiling tiling, VkImageUsageFlags usage,
    DedicatedResourceAllocator* alloc, CommandPool* encode_pool,
    uint32_t queue_family_index) {
    device_ = device;
    if (query_pool_ != nullptr) {
        vkDestroyQueryPool(device_, query_pool_, nullptr);
        query_pool_ = nullptr;
    }

    video_profile_ = encode_profile;
    if (num_images && encode_profile) {
        VkResult result = CreateVideoQueries(num_images, encode_profile);
        if (result != VK_SUCCESS) {
            return 0;
        }
    }

    image_format_ = image_format;
    queue_family_index_ = queue_family_index;
    image_create_info_.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info_.pNext = encode_profile;
    image_create_info_.imageType = VK_IMAGE_TYPE_2D;
    image_create_info_.format = image_format;
    image_create_info_.extent = {max_image_width, max_image_height, 1};
    image_create_info_.mipLevels = 1;
    image_create_info_.arrayLayers = 1;
    image_create_info_.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info_.tiling = tiling;
    image_create_info_.usage = usage;
    image_create_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info_.queueFamilyIndexCount = 1;
    image_create_info_.pQueueFamilyIndices = &queue_family_index_;
    image_create_info_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info_.flags = 0;
    max_bitstream_size_ = ((max_image_width > 3840) ? 8 : 4) * 1024 *
                          1024 /* 4MB or 8MB each for 8k use case */;
    if (num_images) {
        extent_.width = max_image_width;
        extent_.height = max_image_height;
        full_image_size_ = full_image_size;

        return InitFrame(num_images, device_, &image_create_info_, alloc,
                         encode_pool, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 0);
    } else {
        DeInitFramePool();
    }

    return 0;
}
void PictureBuffer::DeInitFramePool() {
    if (query_pool_ != nullptr) {
        vkDestroyQueryPool(device_, query_pool_, nullptr);
        query_pool_ = nullptr;
    }
}
void PictureBuffer::PrepareInputImages(VkCommandBuffer cmd_buffer) {
    for (size_t idx = 0; idx < framebuffer_size_; idx++) {
        InitImageLayout(cmd_buffer, &encode_frame_data_[idx].picture_,
                        VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR);
    }
}
void PictureBuffer::InitReferenceFramePool(uint32_t num_images,
                                           VkFormat image_format,
                                           VkResourceAllocator* alloc) {
    VkImageCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.pNext = video_profile_;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = image_format;
    create_info.extent = {extent_.width, extent_.height, 1};
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;  // DPB ONLY
    create_info.sharingMode =
        VK_SHARING_MODE_CONCURRENT;  // VK_SHARING_MODE_EXCLUSIVE here makes it
                                     // not check for queueFamily
    create_info.queueFamilyIndexCount = 1;
    create_info.pQueueFamilyIndices = &queue_family_index_;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.flags = 0;

    dpb_size_ = num_images;
    for (uint32_t idx = 0; idx < num_images; idx++) {
        RHIImage current_img = alloc->CreateImage(image_create_info_);
        VkImageViewCreateInfo view_create_info =
            MakeImage2DViewCreateInfo(current_img.image, image_format);
        Texture image_view =
            alloc->CreateTexture(current_img, view_create_info);
        Picture ref_pic(current_img, image_view, create_info.initialLayout);
        dpb_.push_back(ref_pic);
    }
}
void PictureBuffer::PrepareReferenceImages(VkCommandBuffer cmd_buffer) {
    for (auto&& tex : dpb_) {
        InitImageLayout(cmd_buffer, &tex, VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR);
    }
}
void PictureBuffer::GetFrameResourcesByIndex(
    int8_t reference_slot_index,
    VkVideoPictureResourceInfoKHR* picture_resources) {
    // picture_resources->imageViewBinding =
}
void PictureBuffer::GetReferenceFrameResourcesByIndex(
    int8_t dpb_slot_idx, VkVideoPictureResourceInfoKHR* picture_resources) {
    Picture* ref_pic = &dpb_[dpb_slot_idx];
    picture_resources->imageViewBinding =
        ref_pic->image_view_.descriptor.imageView;
    picture_resources->codedExtent = extent_;
    picture_resources->codedOffset = {0, 0};
    picture_resources->baseArrayLayer = 0;
}
uint32_t PictureBuffer::InitFrame(
    uint32_t num_images, VkDevice dev,
    const VkImageCreateInfo* image_create_info,
    DedicatedResourceAllocator* alloc, CommandPool* encode_pool,
    VkMemoryPropertyFlags reqs_mem_props, int32_t init_with_pattern,
    VkExternalMemoryHandleTypeFlagBitsKHR export_handle) {
    framebuffer_size_ = num_images;
    VkFenceCreateInfo fence_info;
    ZeroVKStruct(fence_info, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    VkFenceCreateInfo complete_info;
    ZeroVKStruct(complete_info, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    complete_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkSemaphoreCreateInfo sem_info;
    ZeroVKStruct(sem_info, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
    resource_alloc_ = alloc;

    encode_frame_data_.resize(num_images);
    for (uint8_t image_index = 0; image_index < num_images; image_index++) {
        encode_frame_data_[image_index].device_ = dev;
        encode_frame_data_[image_index].extent_ = extent_;
        encode_frame_data_[image_index].queue_family_index_ =
            queue_family_index_;
        encode_frame_data_[image_index].video_profile_ = *video_profile_;
        encode_frame_data_[image_index].picture_.image_ =
            alloc->CreateImage((*image_create_info));
        VkImageViewCreateInfo image_view_create_info =
            MakeImage2DViewCreateInfo(
                encode_frame_data_[image_index].picture_.image_.image,
                image_create_info->format);

        encode_frame_data_[image_index].picture_.image_view_ =
            alloc->CreateTexture(
                encode_frame_data_[image_index].picture_.image_,
                image_view_create_info);
        VkResult result = vkCreateFence(
            dev, &fence_info, nullptr,
            &encode_frame_data_[image_index].frame_complete_fence_);
        result = vkCreateSemaphore(
            dev, &sem_info, nullptr,
            &encode_frame_data_[image_index].frame_encoded_semaphore_);
        result = vkCreateSemaphore(
            dev, &sem_info, nullptr,
            &encode_frame_data_[image_index].frame_producer_done_semaphore_);

        VkBufferCreateInfo out_bitstream_createinfo = MakeBufferCreateInfo(
            max_bitstream_size_, VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR);
        encode_frame_data_[image_index].out_bistream_buffer_ =
            alloc->CreateBuffer(
                out_bitstream_createinfo,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);  // FLAGS - map
                                                            // buffer to host

        VkBufferCreateInfo staging_buffer_createinfo = MakeBufferCreateInfo(
            full_image_size_, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        encode_frame_data_[image_index].input_staging_buffer_ =
            alloc->CreateBuffer(staging_buffer_createinfo,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        encode_frame_data_[image_index].encode_command_buffer_ =
            encode_pool->CreateCommandBuffer();
    }

    return framebuffer_size_;
}

void PictureBuffer::AddRefPicture(uint8_t image_idx, int8_t dpb_index,
                                  uint32_t poc) {
    // uint8_t refCount = m_encodeFrameData[inImageIdx].m_refCount;
    // if (refCount < DECODED_PICTURE_BUFFER_SIZE) {
    //     m_encodeFrameData[inImageIdx].m_RefPics[refCount].m_dpbIdx = dpbIdx;
    //     m_encodeFrameData[inImageIdx].m_RefPics[refCount].m_poc = poc;
    //     m_encodeFrameData[inImageIdx].m_refCount++;
    // }
}
void PictureBuffer::RemoveRefPicture(uint8_t image_idx) {
    // uint8_t refCount = m_encodeFrameData[inImageIdx].m_refCount;
    // if (refCount > 0) {
    //     m_encodeFrameData[inImageIdx].m_RefPics[refCount].m_dpbIdx = -1;
    //     m_encodeFrameData[inImageIdx].m_RefPics[refCount].m_poc = -1;
    //     m_encodeFrameData[inImageIdx].m_refCount--;
    // }
}
int32_t PictureBuffer::ConfigRefPicture(uint8_t dist_anchors,
                                        uint8_t distance_intras,
                                        uint32_t current_poc,
                                        uint8_t current_encode_frame_idx) {
    // if (!m_encodeFrameData[currentEncodeFrameIdx]
    //          .m_usedDpbMask) {  // if mask is not 0 then reset it
    //     for (int32_t i = 0; i < DECODED_PICTURE_BUFFER_SIZE; i++) {
    //         m_encodeFrameData[currentEncodeFrameIdx].m_RefPics[i].m_dpbIdx =
    //         -1; m_encodeFrameData[currentEncodeFrameIdx].m_RefPics[i].m_poc =
    //         -1;
    //     }
    //     m_encodeFrameData[currentEncodeFrameIdx].m_refCount = 0;
    //     m_encodeFrameData[currentEncodeFrameIdx].m_usedDpbMask = 0;
    // }
    // if (!distBetweenAnchors && distanceBetweenIntras == 1) {  // Intra Only
    //     m_encodeFrameData[currentEncodeFrameIdx].m_RefPics[0].m_dpbIdx = 0;
    //     m_encodeFrameData[currentEncodeFrameIdx].m_RefPics[0].m_poc =
    //         currentPoc;
    //     m_encodeFrameData[currentEncodeFrameIdx].m_refCount = 1;
    //     m_encodeFrameData[currentEncodeFrameIdx].m_usedDpbMask += 1;
    //     return 0;
    // } else {
    //     fprintf(stderr, "No support for P abd B frames!\n");
    //     return -1;
    // }
    return 0;
}
VkResult PictureBuffer::CopyToVkImage(uint32_t index, uint32_t buffer_offset,
                                      VkCommandBuffer cmd_buffer) {
    // EncodeFrameData* current_encode_frame_data = &encode_frame_data_[index];
    // Picture* picture = &current_encode_frame_data->picture_;

    // VkImage input_image = picture->image_.image;
    // VkBuffer input_staging =
    //     current_encode_frame_data->input_staging_buffer_.buffer;

    // uint32_t width = image_create_info_.extent.width;
    // uint32_t height = image_create_info_.extent.height;

    // const VkMpFormatInfo* format_info = YcbcrVkFormatInfo(m_imageFormat);

    // // This is to be used for the image memory barriers, if they are needed.
    // VkImageSubresourceRange range = {};
    // range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    // range.baseMipLevel = 0;
    // range.levelCount = 1;
    // range.baseArrayLayer = 0;
    // range.layerCount = 1;

    // VkBufferImageCopy region = {};
    // region.imageSubresource.baseArrayLayer = 0;
    // region.imageSubresource.mipLevel = 0;
    // region.imageSubresource.layerCount = 1;
    // region.imageOffset.x = 0;
    // region.imageOffset.y = 0;
    // region.imageOffset.z = 0;
    // region.imageExtent.depth = 1;

    // std::vector<VkBufferImageCopy> copy_regions;

    // for (uint32_t plane = 0;
    //      plane <= format_info->planesLayout.numberOfExtraPlanes; plane++) {
    //     uint32_t w = 0;
    //     uint32_t h = 0;

    //     if ((plane > 0) &&
    //         format_info->planesLayout.secondaryPlaneSubsampledX) {  // if subsampled on X divide width by 2
    //            w =  (width + 1) / 2;  // add 1 before division in case width is an odd number
    //     } else {
    //         w = width;
    //     }

    //     if ((plane > 0) &&
    //         format_info->planesLayout.secondaryPlaneSubsampledY) {  // if subsampled on Y divide height by 2 
    //           h =  (height + 1) / 2;  // add 1 before division in case height is an odd number
    //     } else {
    //         h = height;
    //     }

    //     region.bufferOffset = buffer_offset;
    //     region.bufferRowLength = w;
    //     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_PLANE_0_BIT
    //                                          << plane;
    //     region.imageExtent.width = w;
    //     region.imageExtent.height = h;

    //     copy_regions.push_back(region);

    //     bufferOffset += w * h;  // w * h is the size of the plane
    // }

    // if (picture->image_layout_ != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    //     // Transition the layout to the desired one.
        
    //     nvvk::cmdBarrierImageLayout(cmdBuf, inputImage, picture->m_imageLayout,
    //                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                                 range);
    // }

    // vkCmdCopyBufferToImage(cmdBuf, inputStaging, inputImage,
    //                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                        (uint32_t)copyRegions.size(), copyRegions.data());

    // if (picture->m_imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    //     // Restore the original image layout
    //     nvvk::cmdBarrierImageLayout(cmdBuf, inputImage,
    //                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                                 picture->m_imageLayout, range);
    // }

    return VkResult();
}
EncodeFrameData* PictureBuffer::GetEncodeFrameData(uint32_t index) {
    return &encode_frame_data_[index];
}
void PictureBuffer::InitImageLayout(VkCommandBuffer cmd_buffer,
                                    Picture* picture, VkImageLayout layout) {
    VkImageSubresourceRange range = {};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;
    VkImageUtil::CmdBarrierImageLayout(cmd_buffer, picture->image_.image,
                                       picture->image_layout_, layout, range);
    picture->image_layout_ = layout;
}
}  // namespace TSEngine