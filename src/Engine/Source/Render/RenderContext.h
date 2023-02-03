#pragma once
#include <map>
#include <memory>
#include <functional>


#include "Vulkan/CommandPool.h"
#include "Vulkan/ResourceAllocator.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VkContext.h"

namespace toystation {

struct RenderContextCreateInfo {
    enum RenderMode { RENDER_LOCAL, RENDER_REMOTE, RENDER_INVALID };

    RenderMode mode;
    static std::map<RenderMode, std::string> kRenderModeStr;
};

template <typename T>
std::string ToStr(T type) {
    return "";
}

template <>
std::string ToStr(RenderContextCreateInfo::RenderMode mode);
class RenderContext {
public:
    void Initialize(RenderContextCreateInfo& info);

    std::shared_ptr<VkContext> GetContext() const;
    std::shared_ptr<SwapChainBase> GetSwapchain() const;
    std::shared_ptr<VkResourceAllocator> GetAllocator() const;
    std::shared_ptr<CommandPool> GetCommandPool() const;

private:
    std::shared_ptr<VkContext> vkctx_;
    std::shared_ptr<SwapChainBase> swapchain_;
    std::shared_ptr<DedicatedResourceAllocator> allocator_;
    std::shared_ptr<CommandPool> cmd_pool_;
};
enum RenderFrameType { FRAME_RGB,
    FRAME_RGBA };

class RenderFrame {
public:
    virtual ~RenderFrame() {}
    virtual unsigned char* Data()const { return nullptr; }
    virtual unsigned int Width()const { return 0; }
    virtual unsigned int Height()const { return 0; }
    virtual RenderFrameType Type() { return RenderFrameType::FRAME_RGB; }
};

class RenderEvent {
public:
    static std::function<void(const RenderFrame&)> OnRenderDone;
};
}  // namespace toystation