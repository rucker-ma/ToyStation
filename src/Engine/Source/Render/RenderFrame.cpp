//
// Created by ma on 2023/3/13.
//

#include "RenderFrame.h"

#include "Device/Cuda/CudaPlatform.h"
#include "Vulkan/VkImageUtil.h"

namespace toystation {
RenderFrameNV12::RenderFrameNV12(std::shared_ptr<RenderContext> context,
                                 const RHIImage& compy, const RHIImage& compuv)
    : context_(context) {
    VkRect2D* rect = context->GetSwapchain()->GetScissor();

    width_ = rect->extent.width;
    //width_=2048;
    height_ = rect->extent.height;

    ReadRHIImage(buffer_y_, compy, width_ * height_,
                 VkExtent2D{width_, height_});
    ReadRHIImage(buffer_uv_, compuv, width_ * height_ / 2,
                 VkExtent2D{width_, height_ / 2});

    cuda_mem_y_ = CudaExternalMemory::FromVulkanExternalMemory(
        context->GetContext(), buffer_y_);
    cuda_mem_uv_ = CudaExternalMemory::FromVulkanExternalMemory(
        context->GetContext(), buffer_uv_);
}
RenderFrameNV12::~RenderFrameNV12() {
    context_->GetAllocator()->Destroy(buffer_y_);
    context_->GetAllocator()->Destroy(buffer_uv_);

}
unsigned char* toystation::RenderFrameNV12::DataY() const {
    return static_cast<unsigned char*>(cuda_mem_y_->Data());
}
unsigned char* RenderFrameNV12::DataUV() const {
    return static_cast<unsigned char*>(cuda_mem_uv_->Data());
}
unsigned int RenderFrameNV12::Width() const { return width_; }
unsigned int RenderFrameNV12::Height() const { return height_; }
RenderFrameType RenderFrameNV12::Type() const {
    return RenderFrameType::FRAME_NV12;
}
void RenderFrameNV12::ReadRHIImage(RHIBuffer& buf, const RHIImage& img,
                                   VkDeviceSize mem_size, VkExtent2D img_size) {
    buf = context_->GetAllocator()->CreateExternalBuffer(
        mem_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer();
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageUtil::CmdBarrierImageLayout(cmd, img.image, VK_IMAGE_LAYOUT_GENERAL,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       sub_range);
    VkBufferImageCopy copy_region{};
    copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.imageExtent = {img_size.width, img_size.height, 1};
    vkCmdCopyImageToBuffer(cmd, img.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           buf.buffer, 1, &copy_region);

    VkImageUtil::CmdBarrierImageLayout(cmd, img.image,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       VK_IMAGE_LAYOUT_GENERAL, sub_range);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}

}  // namespace toystation