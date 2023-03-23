#include "RenderSystem.h"

#include "Base/Global.h"
#include "Base/Thread.h"
#include "RenderDocCapture.h"
#include "RenderObject.h"
#include "ToyEngine.h"
#include "ToyEngineSetting.h"

namespace toystation {

RenderGlobalData RenderSystem::kRenderGlobalData = {};

void RenderGlobalData::AddRenderObject(std::shared_ptr<TObject> obj) {
    int id = obj->GetID();
    std::shared_ptr<RenderObject> render_object =
        std::make_shared<RenderObject>(id);

    //TODO:添加实现细节
    render_resource->render_objects_.insert(std::make_pair(id, render_object));
}

void RenderSystem::Initialize() {
    LogDebug("RenderSystem Initialize..");
    RenderDocCapture::Instance().Init();
    // vulkan环境初始化,根据渲染模式的不同配置不同的instance,device扩展
    RenderContextCreateInfo context_create_info{};
    context_create_info.mode = RenderContextCreateInfo::RENDER_REMOTE;
    LogDebug("set render mode : " + ToStr(context_create_info.mode));
    kRenderGlobalData.render_context = std::make_shared<RenderContext>();
    kRenderGlobalData.render_context->Initialize(context_create_info);
    kRenderGlobalData.render_resource = std::make_shared<RenderResource>();
    render_pipeline_ = std::make_shared<RenderPipeline>();
    render_pipeline_->Initialize();

    if (ToyEngineSetting::Instance().GetUseHWAccel()) {
        frame_convert_ = std::make_shared<FrameConvertNV12Pass>();
    } else {
        frame_convert_ = std::make_shared<FrameConvertYCrCbPass>();
    }
    auto init_info = render_pipeline_->GetRenderPassInitInfo();
    frame_convert_->Initialize(init_info);

    Global::SetRenderThread(std::thread([this] { Run(); }));
}

void RenderSystem::Tick() {
    render_pipeline_->Tick();
    ProcessEvent();
}
void RenderSystem::Run() {
    ThreadUtil::SetCurrentThreadName("render_thread");
    std::shared_ptr<Msg> msg;
    // 计算渲染帧率
    Calculagraph calcu("Render FrameRate", 100, 1000);
    calcu.OnEnd = [](double result, long long) {
        LogInfo("Render FrameRate: " +
                std::to_string(static_cast<int>(result)));
    };
    calcu.Start();
    while (true) {
        if (kMesssageQueue.Get(msg)) {
            if (msg->GetID() == kRenderMessageID) {
                auto* render_msg = dynamic_cast<RenderMessage*>(msg.get());
                if (render_msg) {
                    switch (render_msg->GetPayload().Action()) {
                        case RenderAction::Render_General:
                            Tick();
                            break;
                        case RenderAction::Render_RenderDocCapture:
                            RenderDocCapture::Instance().StartCapture();
                            Tick();
                            RenderDocCapture::Instance().EndCapture();
                            break;
                    }
                    calcu.Step();
                }
            }
            if (msg->GetID() == kRenderTaskID) {
                // 当前来自于level读取object后传递到渲染线程执行将数据拷贝到gpu
                std::shared_ptr<Task> task =
                    std::dynamic_pointer_cast<Task>(msg);
                if (task) {
                    task->Run();
                }
            }
        }
    }
}
void RenderSystem::ProcessEvent() {
    if (RenderEvent::OnRenderDone) {
        frame_convert_->Draw();
        kMesssageQueue.Post(
            kTransferThread.get_id(),
            std::make_shared<DataMsg<RenderFrame>>(
                kTransferMessageID, frame_convert_->GetConvertFrame()));
    }
}

}  // namespace toystation