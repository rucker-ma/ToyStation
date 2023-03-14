#pragma once
#include "Base/MessageQueue.h"
#include "Base/Calculagraph.h"

#include "RenderContext.h"
#include "RenderPipeline.h"
#include "RenderResource.h"
#include "ConvertPass.h"
#include "Vulkan/VkContext.h"

namespace toystation {

enum class RenderAction{
    Render_General,
    Render_RenderDocCapture
};

class RenderPayload {
public:
    RenderPayload() {action_ = RenderAction::Render_General;}
    RenderPayload(RenderAction action):action_(action){}
    RenderAction Action(){return action_;}
private:
    RenderAction action_;
};

using RenderMessage = DataMsg<RenderPayload>;

/// @brief 存储渲染相关全局资源，如vkdevice,vkinstance等
struct RenderGlobalData {
    std::shared_ptr<RenderContext> render_context;
    std::shared_ptr<RenderResource> render_resource;
};
class TS_CPP_API RenderSystem {
public:
    void Initialize();
    void Tick();

    static RenderGlobalData kRenderGlobalData;

private:
    void Run();
    void ProcessEvent();
private:
    std::shared_ptr<RenderPipeline> render_pipeline_;
    std::shared_ptr<FrameConvertPass> frame_convert_;
};

}  // namespace toystation