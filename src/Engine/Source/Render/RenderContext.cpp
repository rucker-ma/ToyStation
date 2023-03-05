#include "RenderContext.h"

#include "File/FileUtil.h"
#include "Vulkan/VkImageUtil.h"
namespace toystation {

std::function<void(std::shared_ptr<RenderFrame>)> RenderEvent::OnRenderDone;

std::map<RenderContextCreateInfo::RenderMode, std::string>
    RenderContextCreateInfo::kRenderModeStr = {
        TYPE_PAIR(RenderContextCreateInfo::RENDER_LOCAL),
        TYPE_PAIR(RenderContextCreateInfo::RENDER_REMOTE),
        TYPE_PAIR(RenderContextCreateInfo::RENDER_INVALID)};

template <>
std::string ToStr(RenderContextCreateInfo::RenderMode mode) {
    auto iter = RenderContextCreateInfo::kRenderModeStr.find(mode);
    if (iter != RenderContextCreateInfo::kRenderModeStr.end()) {
        return RenderContextCreateInfo::kRenderModeStr[mode];
    }
    return std::string();
}

void RenderContext::Initialize(RenderContextCreateInfo& info) {
    vkctx_ = std::make_shared<VkContext>();
    VkContextCreateInfo context_create_info;
    if (info.mode == RenderContextCreateInfo::RENDER_REMOTE) {
        // context_create_info.AddDeviceExtension(
        //     VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME);
        // context_create_info.AddDeviceExtension(
        //     VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
        // context_create_info.AddDeviceExtension(
        //     VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
        // context_create_info.AddDeviceExtension(
        //     VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME);
        swapchain_ = std::make_shared<SwapChainOffline>();
    }
    if (info.mode == RenderContextCreateInfo::RENDER_LOCAL) {
        context_create_info.AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        swapchain_ = std::make_shared<SwapChain>();

        // for local render, we also need `VK_KHR_surface` and
        // `VK_KHR_win32_surface` or other platform surface,now not test
    }

    vkctx_->Init(context_create_info);
    allocator_ = std::make_shared<DedicatedResourceAllocator>();
    allocator_->Init(vkctx_.get());
    cmd_pool_ = std::make_shared<CommandPool>();
    cmd_pool_->Init(vkctx_->GetDevice(),
                    vkctx_->GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT));
    
    camera_ = std::make_shared<RenderCamera>();
}

std::shared_ptr<VkContext> RenderContext::GetContext() const { return vkctx_; }

std::shared_ptr<SwapChainBase> RenderContext::GetSwapchain() const {
    return swapchain_;
}
std::shared_ptr<VkResourceAllocator> RenderContext::GetAllocator() const {
    return allocator_;
}
std::shared_ptr<CommandPool> RenderContext::GetCommandPool() const {
    return cmd_pool_;
}

std::shared_ptr<RenderCamera> RenderContext::GetCamera() const {
    return camera_;
}

void RenderContext::SaveToImage(std::string filename, RHIImage& image,
                                VkImageLayout layout, VkFormat format,
                                std::shared_ptr<RenderContext> context) {
    VkRect2D* rect = context->GetSwapchain()->GetScissor();
    VkDeviceSize mem_size = rect->extent.width * rect->extent.height * 4;
    Buffer buf = context->GetAllocator()->CreateBuffer(
        mem_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkCommandBuffer cmd = context->GetCommandPool()->CreateCommandBuffer();

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageUtil::CmdBarrierImageLayout(cmd, image.image, layout,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       sub_range);
    VkBufferImageCopy copy_region{};
    copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.imageExtent = {rect->extent.width, rect->extent.height, 1};
    vkCmdCopyImageToBuffer(cmd, image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf.buffer, 1,
                           &copy_region);
    VkImageUtil::CmdBarrierImageLayout(cmd, image.image,
                                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                       layout, sub_range);
    context->GetCommandPool()->SubmitAndWait(cmd);
    void* data = context->GetAllocator()->Map(buf);

    FileUtil::WriteBmp(filename.c_str(), static_cast<unsigned char*>(data),
                       rect->extent.width, rect->extent.height);

    context->GetAllocator()->UnMap(buf);
    context->GetAllocator()->Destroy(buf);
}

RenderFrameImpl::RenderFrameImpl(std::shared_ptr<RenderContext> context,
                                 const RHIImage& img)
    : context_(context) {
    VkRect2D* rect = context_->GetSwapchain()->GetScissor();
    VkDeviceSize mem_size = rect->extent.width * rect->extent.height * 4;
    width_ = rect->extent.width;
    height_ = rect->extent.height;

    buf_ = context_->GetAllocator()->CreateBuffer(
        mem_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer();
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageUtil::CmdBarrierImageLayout(
        cmd, img.image, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, sub_range);
    VkBufferImageCopy copy_region{};
    copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.imageExtent = {rect->extent.width, rect->extent.height, 1};
    vkCmdCopyImageToBuffer(cmd, img.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           buf_.buffer, 1, &copy_region);

    VkImageUtil::CmdBarrierImageLayout(
        cmd, img.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, sub_range);
    context_->GetCommandPool()->SubmitAndWait(cmd);
    data_ = static_cast<unsigned char*>(context_->GetAllocator()->Map(buf_));
}

RenderFrameImpl::~RenderFrameImpl() {
    data_ = nullptr;
    context_->GetAllocator()->UnMap(buf_);
    context_->GetAllocator()->Destroy(buf_);
}

unsigned char* RenderFrameImpl::Data() const { return data_; }

unsigned int RenderFrameImpl::Width() const { return width_; }

unsigned int RenderFrameImpl::Height() const { return height_; }

RenderFrameType RenderFrameImpl::Type() const {
    return RenderFrameType::FRAME_RGBA;
}

RenderFrameYCbCr::RenderFrameYCbCr(std::shared_ptr<RenderContext> context,
                                   const RHIImage& compy,
                                   const RHIImage& compcb,
                                   const RHIImage& compcr)
    : context_(context) {
    VkRect2D* rect = context_->GetSwapchain()->GetScissor();
    VkDeviceSize mem_size = rect->extent.width * rect->extent.height * 4;
    width_ = rect->extent.width;
    height_ = rect->extent.height;

    datay_ = ReadRHIImage(bufy_, compy, width_ * height_,
                          VkExtent2D{width_, height_});
    datau_ = ReadRHIImage(bufu_, compcb, width_ * height_ / 4,
                          VkExtent2D{width_ / 2, height_ / 2});
    datav_ = ReadRHIImage(bufv_, compcr, width_ * height_ / 4,
                          VkExtent2D{width_ / 2, height_ / 2});
}

unsigned char* RenderFrameYCbCr::ReadRHIImage(Buffer& buf, const RHIImage& img,
                                              VkDeviceSize mem_size,
                                              VkExtent2D img_size) {
    buf = context_->GetAllocator()->CreateBuffer(
        mem_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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
    return static_cast<unsigned char*>(context_->GetAllocator()->Map(buf));
}

RenderFrameYCbCr::~RenderFrameYCbCr() {
    datay_ = nullptr;
    datau_ = nullptr;
    datav_ = nullptr;
    context_->GetAllocator()->UnMap(bufy_);
    context_->GetAllocator()->Destroy(bufy_);

    context_->GetAllocator()->UnMap(bufu_);
    context_->GetAllocator()->Destroy(bufu_);

    context_->GetAllocator()->UnMap(bufv_);
    context_->GetAllocator()->Destroy(bufv_);
}

unsigned char* RenderFrameYCbCr::Data() const { return nullptr; }

unsigned char* RenderFrameYCbCr::DataY() const { return datay_; }
unsigned char* RenderFrameYCbCr::DataCb() const { return datau_; }
unsigned char* RenderFrameYCbCr::DataCr() const { return datav_; }

unsigned int RenderFrameYCbCr::Width() const { return width_; }

unsigned int RenderFrameYCbCr::Height() const { return height_; }

RenderFrameType RenderFrameYCbCr::Type() const {
    return RenderFrameType::FRAME_YCbCr;
}

}  // namespace toystation