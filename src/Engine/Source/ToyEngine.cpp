#include "ToyEngine.h"
#include "Base/Global.h"

namespace toystation {

// 16ms = 1000ms / 60pfs
constexpr int kTickIntervalMS = 16;

ToyEngine kEngine;

void ToyEngine::Init(){
    render_system_.Initialize();
    transfer_system_.Initialize();
    input_system_.Initialize();
}
void ToyEngine::Run() {
    while (true) {
        SystemTick();
        RenderTick();
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickIntervalMS));
    }
}
void ToyEngine::SystemTick() {
    input_system_.Tick();
}
void ToyEngine::RenderTick(){
    RenderAction flag = RenderAction::Render_General;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (!render_flags_.empty()){
            flag = render_flags_.front();
            render_flags_.pop_front();
        }
    }
    kMesssageQueue.Post(kRendThread.get_id(),std::make_shared<RenderMessage>(kRenderMessageID,flag));
}
InputSystem& ToyEngine::GetInputSystem() { return input_system_; }
void ToyEngine::PushRenderFlag(RenderAction flag) {
    std::lock_guard<std::mutex> lock(mtx_);
    render_flags_.push_back(flag);
}
}  // namespace toystation
