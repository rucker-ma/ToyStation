#pragma once

#include "Base/Macro.h"
#include "Base/MessageQueue.h"

#include "Render/RenderSystem.h"
#include "Transfer/TransferSystem.h"
#include "Input/InputSystem.h"
#include "Framework/WorldManager.h"

namespace toystation {
CLASS()
class TS_CPP_API ToyEngine {
public:
    ToyEngine(){};
    FUNCTION(CSHARP)
    void Init();
    FUNCTION(CSHARP)
    void Run();
    void PushRenderFlag(RenderAction flag);
    InputSystem& GetInputSystem();
private:
    void SystemTick();
    void RenderTick();
private:
    TransferSystem transfer_system_;
    RenderSystem render_system_;
    InputSystem input_system_;
    WorldManager world_manager_;
    std::deque<RenderAction> render_flags_;
    std::mutex mtx_;
};

extern ToyEngine kEngine;
}  // namespace toystation
