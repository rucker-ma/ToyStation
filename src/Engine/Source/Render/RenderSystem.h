#pragma once
#include "Base/MessageQueue.h"
#include "Base/Calculagraph.h"

#include "RenderContext.h"
#include "RenderPipeline.h"
#include "Vulkan/VkContext.h"

namespace toystation {

class RenderPayload {
public:
    RenderPayload() = default;

private:
    int placeholder;
};

using RenderMessage = DataMsg<RenderPayload>;

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
    void Run();

private:
    std::shared_ptr<RenderPipeline> render_pipeline_;
};

}  // namespace toystation