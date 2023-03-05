#pragma once

#include "Base/Macro.h"
#include "Base/MessageQueue.h"

#include "Render/RenderSystem.h"
#include "Transfer/TransferSystem.h"
#include "Input/InputSystem.h"

namespace toystation {
CLASS()
class TS_CPP_API ToyEngine {
public:
    ToyEngine(){};

    FUNCTION(CSHARP)
    void Init();
    FUNCTION(CSHARP)
    void Run();

    InputSystem& GetInputSystem();
private:
    void SystemTick();
private:
    TransferSystem transfer_system_;
    RenderSystem render_system_;
    InputSystem input_system_;
};

extern ToyEngine kEngine;
}  // namespace toystation
