#include "ToyEngine.h"
#include "Base/Global.h"

namespace toystation {

constexpr int kTickIntervalMS = 30;


void ToyEngine::Init() {
    render_system_.Initialize();
    transfer_system_.Initialize();
}
void ToyEngine::Run() {
    while (true) {
        kMesssageQueue.Post(kRendThread.get_id(),std::make_shared<RenderMessage>(kRenderMessageID));
        std::this_thread::sleep_for(std::chrono::milliseconds(kTickIntervalMS));
    }
}
}  // namespace toystation
