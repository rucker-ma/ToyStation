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

    render_pipeline_ = std::make_shared<RenderPipeline>();
    render_pipeline_->Initialize();

    Global::SetRenderThread(std::thread([this] { Run(); }));
}
void RenderSystem::Tick() { render_pipeline_->Tick(); }
void RenderSystem::Run() {
    std::shared_ptr<Msg> msg;
    while (true) {
        if (kMesssageQueue.Get(msg)) {
            if (msg->GetID() == kRenderMessageID) {
                auto* render_msg = dynamic_cast<RenderMessage*>(msg.get());
                if (render_msg) {
                    LogDebug("RenderSystem Tick");
                    Tick();
                }
            }
        }
    }
}
}  // namespace toystation