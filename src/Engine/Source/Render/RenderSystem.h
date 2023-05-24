#pragma once
#include "Base/MessageQueue.h"
#include "Base/Calculagraph.h"
#include "Vulkan/VkContext.h"

#include "RenderContext.h"
#include "RenderPipeline.h"
#include "ConvertPass.h"
#include "RenderGlobalData.h"


namespace toystation {

enum class RenderAction{
    Render_General,
    Render_RenderDocCapture,
    Render_UpdatePipeline
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
class TS_CPP_API RenderSystem {
public:
    void Initialize();
    void PostInitialize();
    void Tick();

    static RenderGlobalData kRenderGlobalData;
private:
    void Run();
    void ProcessEvent();
    void RegisterCapture();
private:
    std::shared_ptr<RenderPipeline> render_pipeline_;
    std::shared_ptr<FrameConvertPass> frame_convert_;
    bool connected_; //anyone connect to server
    bool initialize_phase_;
    bool capture_frame_;
};

}  // namespace toystation