#include "RenderSystem.h"

#include "Base/Global.h"
#include "Base/Thread.h"
#include "Base/Time.h"

#include "RenderDocCapture.h"
#include "RenderObject.h"
#include "ToyEngine.h"
#include "ToyEngineSetting.h"


namespace toystation {

RenderGlobalData RenderSystem::kRenderGlobalData = {};

void RenderSystem::Initialize() {
    LogDebug("RenderSystem Initialize..");
    initialize_phase_ = true;
    RenderDocCapture::Instance().Init();
    // vulkan环境初始化,根据渲染模式的不同配置不同的instance,device扩展
    RenderContextCreateInfo context_create_info{};
    context_create_info.mode = RenderContextCreateInfo::RENDER_REMOTE;
    LogDebug("set render mode : " + ToStr(context_create_info.mode));
    kRenderGlobalData.render_context = std::make_shared<RenderContext>();
    kRenderGlobalData.render_context->Initialize(context_create_info);

    kRenderGlobalData.render_resource = std::make_shared<RenderResource>();
    kRenderGlobalData.render_resource->Initialize(kRenderGlobalData.render_context);

    render_pipeline_ = std::make_shared<RenderPipeline>();
    render_pipeline_->Initialize();

    if (ToyEngineSetting::Instance().GetUseHWAccel()) {
        frame_convert_ = std::make_shared<FrameConvertNV12Pass>();
    } else {
        frame_convert_ = std::make_shared<FrameConvertYCrCbPass>();
    }
    auto init_info = render_pipeline_->GetRenderPassInitInfo();
    frame_convert_->Initialize(init_info);
    connected_ = false;
    Global::SetRenderThread(std::thread([this] { Run(); }));
}
void RenderSystem::PostInitialize(){
    initialize_phase_ = false;
}
void RenderSystem::Tick() {
    if(kEngine.GetTransferSystem().AnyConnected()){
        if(!connected_){
            connected_ = true;
            debug::TimePiling::Instance().ClearCount();
        }
    }else{
        if(connected_){
            connected_ =false;
            debug::TimePiling::Instance().ClearCount();
        }
    }
    debug::TimePiling::Instance().Mark("render start",0);
    render_pipeline_->Tick();
    if (RenderEvent::OnRenderDone&& connected_) {
        frame_convert_->Draw();
        debug::TimePiling::Instance().Mark("render end",10);

        kMesssageQueue.Post(
            kTransferThread.get_id(),
            std::make_shared<DataMsg<RenderFrame>>(
                kTransferMessageID, frame_convert_->GetConvertFrame()));
    }else{
        debug::TimePiling::Instance().Mark("render end",10);
        std::string render_info = debug::TimePiling::Instance().GetMarksInfo();
        if(!render_info.empty()){
            LogInfo("frame life cycle: \n" + render_info);
        }
    }
}
void RenderSystem::Run() {
    ThreadUtil::SetCurrentThreadName("render_thread");
    std::shared_ptr<Msg> msg;
    while(initialize_phase_){
        if (kMesssageQueue.Wait(msg,10)){
            if (msg->GetID() == kRenderTaskID) {
                // 当前来自于level读取object后传递到渲染线程执行将数据拷贝到gpu
                std::shared_ptr<TaskMsg> task =
                    std::dynamic_pointer_cast<TaskMsg>(msg);
                if (task) {
                    task->Run();
                }
            }
        }
    }

    render_pipeline_->PreRender();
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
                            if(capture_frame_) {
                                RenderDocCapture::Instance().StartCapture();
                                Tick();
                                RenderDocCapture::Instance().EndCapture();
                                capture_frame_ = false;
                            }else{
                                Tick();
                            }
                            break;
//                        case RenderAction::Render_RenderDocCapture:
//                            RenderDocCapture::Instance().StartCapture();
//                            Tick();
//                            RenderDocCapture::Instance().EndCapture();
//                            break;
                        case RenderAction::Render_UpdatePipeline:
                            render_pipeline_->MarkUpdateShader();
                            Tick();
                            break ;
                    }
                    calcu.Step();
                }
            }
            if (msg->GetID() == kRenderTaskID) {
                // 当前来自于level读取object后传递到渲染线程执行将数据拷贝到gpu
                std::shared_ptr<TaskMsg> task =
                    std::dynamic_pointer_cast<TaskMsg>(msg);
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
void RenderSystem::RegisterCapture(){
    auto response = std::make_shared<CustomInputEventResponse>();
    response->name = "capture";
    response->handler = [this](Json::Value value){
        capture_frame_ = true;
    };
}
}  // namespace toystation