#pragma once

#include "Base/Macro.h"
#include "Base/MessageQueue.h"

#include "Render/RenderSystem.h"
#include "Transfer/TransferSystem.h"
#include "Input/InputSystem.h"
#include "Compiler/ShaderCompilerSystem.h"
#include "Framework/WorldManager.h"

namespace toystation {
CLASS()
class TS_CPP_API ToyEngine {
public:
    ToyEngine(){};
    //device environment init
    void PreInit();
    //system init
    void Init();
    void PostInit();
    void Run();
    void PushRenderFlag(RenderAction flag);
    InputSystem& GetInputSystem();
    TransferSystem& GetTransferSystem();
    WorldManager& GetWorldManager();
    ShaderCompilerSystem& GetShaderSystem();
    RenderSystem& GetRenderSystem();
private:
    void SystemTick();
    void RenderTick();
private:
    TransferSystem transfer_system_;
    RenderSystem render_system_;
    InputSystem input_system_;
    WorldManager world_manager_;
    ShaderCompilerSystem shader_system_;
    std::deque<RenderAction> render_flags_;
    std::mutex mtx_;
};

extern ToyEngine kEngine;
}  // namespace toystation
