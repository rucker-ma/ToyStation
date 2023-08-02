#include "ToyEngine.h"

#include "Base/Global.h"
#include "Framework/TObject.h"
#include "Device/Cuda/CudaPlatform.h"
#include "ToyEngineSetting.h"


namespace toystation {

ToyEngine kEngine;

void ToyEngine::PreInit(){
    CudaPlatform::Instance();
    ToyEngineSetting::Instance().SetUseHWAccel(true);
}
void ToyEngine::Init() {
    shader_system_.Initialize();
    render_system_.Initialize();
    transfer_system_.Initialize();
    input_system_.Initialize();
    world_manager_.Initialize();
    scripts_system_.Initialize();
}
void ToyEngine::PostInit(){
    world_manager_.PostInitialize();
    render_system_.PostInitialize();
}
void ToyEngine::Run() {
    while (true) {
        auto pt_start = std::chrono::system_clock::now();
        SystemTick();
        RenderTick();
        auto pt_end = std::chrono::system_clock::now();
        int milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(pt_end-pt_start).count();
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickIntervalMS-milliseconds));
    }
}
void ToyEngine::SystemTick() {
    input_system_.Tick();
    world_manager_.Tick();
}
void ToyEngine::RenderTick() {
    RenderAction flag = RenderAction::Render_General;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!render_flags_.empty()) {
            flag = render_flags_.front();
            render_flags_.pop_front();
        }
    }
    kMesssageQueue.Post(kRendThread.get_id(), std::make_shared<RenderMessage>(
                                                  kRenderMessageID, flag));
}
InputSystem& ToyEngine::GetInputSystem() { return input_system_; }
TransferSystem& ToyEngine::GetTransferSystem() { return transfer_system_; }
WorldManager& ToyEngine::GetWorldManager(){
    return world_manager_;
}
ShaderCompilerSystem& ToyEngine::GetShaderSystem(){
    return shader_system_;
}
RenderSystem& ToyEngine::GetRenderSystem(){
    return render_system_;
}
void ToyEngine::PushRenderFlag(RenderAction flag) {
    std::lock_guard<std::mutex> lock(mtx_);
    render_flags_.push_back(flag);
}
}  // namespace toystation
