#pragma once

#include "Base/Macro.h"
#include "Base/MessageQueue.h"

#include "Render/RenderSystem.h"
#include "Transfer/TransferSystem.h"

namespace toystation {
CLASS()
class TS_CPP_API ToyEngine {
public:
    ToyEngine(){};

    FUNCTION(CSHARP)
    void Init();
    FUNCTION(CSHARP)
    void Run();

private:
    TransferSystem transfer_system_;
    RenderSystem render_system_;
};
}  // namespace toystation
