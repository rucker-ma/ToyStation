#pragma once

#include "Base/Macro.h"
#include "Base/MessageQueue.h"

#include "Render/RenderSystem.h"
#include "Transfer/TransferSystem.h"
#include "Input/InputSystem.h"
#include "Compiler/ShaderCompilerSystem.h"
#include "Framework/WorldManager.h"
#include "Scripts/ScriptsSystem.h"

namespace toystation {
class ToyEngine {
    GENERATE_BODY(ToyEngine)
public:
    ToyEngine(){};
    //device environment init
    void PreInit();
    //system init
    void Init();
    void PostInit();
    void Run();
    void PushRenderFlag(RenderAction flag);
    TransferSystem& GetTransferSystem();
    WorldManager& GetWorldManager();
    SKIP_GENERATE(
    InputSystem& GetInputSystem();
    ShaderCompilerSystem& GetShaderSystem();
    )
    RenderSystem& GetRenderSystem();
private:
    void SystemTick();
    void RenderTick();
private:
    TransferSystem transfer_system_;
    RenderSystem render_system_;

    WorldManager world_manager_;
    SKIP_GENERATE(
    InputSystem input_system_;
    ShaderCompilerSystem shader_system_;
    ScriptsSystem scripts_system_;
    std::mutex mtx_;
    )
    std::deque<RenderAction> render_flags_;
};

extern ToyEngine kEngine;
}  // namespace toystation
