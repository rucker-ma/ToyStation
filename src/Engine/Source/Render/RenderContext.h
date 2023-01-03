#pragma once
#include <map>
#include <memory>

#include "Vulkan/SwapChain.h"
#include "Vulkan/VkContext.h"
#include "Vulkan/ResourceAllocator.h"
#include "Vulkan/CommandPool.h"

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
    std::shared_ptr<VkResourceAllocator> GetAllocator()const;
    std::shared_ptr<CommandPool> GetCommandPool()const;
private:
    std::shared_ptr<VkContext> vkctx_;
    std::shared_ptr<SwapChainBase> swapchain_;
    std::shared_ptr<DedicatedResourceAllocator> allocator_;
    std::shared_ptr<CommandPool> cmd_pool_;
};
}  // namespace toystation