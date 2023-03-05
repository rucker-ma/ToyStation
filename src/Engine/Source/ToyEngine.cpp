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
        kMesssageQueue.Post(kRendThread.get_id(),std::make_shared<RenderMessage>(kRenderMessageID));
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickIntervalMS));
    }
}
void ToyEngine::SystemTick() {
    input_system_.Tick();
}
InputSystem& ToyEngine::GetInputSystem() { return input_system_; }
}  // namespace toystation
