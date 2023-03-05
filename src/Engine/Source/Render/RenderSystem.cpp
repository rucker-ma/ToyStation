#include "RenderSystem.h"

#include "Base/Global.h"

namespace toystation {

RenderGlobalData RenderSystem::kRenderGlobalData = {};

void RenderSystem::Initialize() {
    LogDebug("RenderSystem Initialize..");
    // vulkan环境初始化,根据渲染模式的不同配置不同的instance,device扩展
    RenderContextCreateInfo context_create_info{};
    context_create_info.mode = RenderContextCreateInfo::RENDER_REMOTE;
    LogDebug("set render mode : " + ToStr(context_create_info.mode));
    kRenderGlobalData.render_context = std::make_shared<RenderContext>();
    kRenderGlobalData.render_context->Initialize(context_create_info);
    kRenderGlobalData.render_resource = std::make_shared<RenderResource>();
    render_pipeline_ = std::make_shared<RenderPipeline>();
    render_pipeline_->Initialize();

    Global::SetRenderThread(std::thread([this] { Run(); }));
}
void RenderSystem::Tick() {
    render_pipeline_->Tick();
    ProcessEvent();
}
void RenderSystem::Run() {
    std::shared_ptr<Msg> msg;
    // 计算渲染帧率
    Calculagraph calcu("Render FrameRate", 100, 1000);
    calcu.OnEnd = [this](double result, long long) {
        LogInfo("Render FrameRate: " +
                std::to_string(static_cast<int>(result)));
    };
    calcu.Start();
    while (true) {
        if (kMesssageQueue.Get(msg)) {
            if (msg->GetID() == kRenderMessageID) {
                auto* render_msg = dynamic_cast<RenderMessage*>(msg.get());
                if (render_msg) {
                    Tick();
                    calcu.Step();
                }
            }
        }
    }
}
void RenderSystem::ProcessEvent() {
    if (RenderEvent::OnRenderDone) {
        kMesssageQueue.Post(kTransferThread.get_id(),
                            std::make_shared<DataMsg<RenderFrame>>(
                                kTransferMessageID,
                                static_cast<RenderFrame*>(new RenderFrameYCbCr(
                                    kRenderGlobalData.render_context,
                                    kRenderGlobalData.render_resource
                                        ->convert_pass_resource_.comp_y,
                                    kRenderGlobalData.render_resource
                                        ->convert_pass_resource_.comp_cb,
                                    kRenderGlobalData.render_resource
                                        ->convert_pass_resource_.comp_cr))));
    }
}
}  // namespace toystation