#include "RenderContext.h"

namespace toystation {

std::function<void(const RenderFrame&)> RenderEvent::OnRenderDone;

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
}  // namespace toystation