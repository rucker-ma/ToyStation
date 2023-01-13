#pragma once
#include "RenderContext.h"
#include "RenderPipeline.h"
#include "Vulkan/VkContext.h"

namespace toystation {

/// @brief 存储渲染相关全局资源，如vkdevice,vkinstance等
struct RenderGlobalData {
    std::shared_ptr<RenderContext> render_context;
};
class TS_CPP_API RenderSystem {
public:
    void Initialize();
    void Tick();

    static RenderGlobalData kRenderGlobalData;

private:
    std::shared_ptr<RenderPipeline> render_pipeline_;
};


}  // namespace toystation