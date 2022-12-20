#pragma once

#include "Base/Macro.h"
#include "Render/Render.h"
namespace TSEngine {
CLASS(SINGLETON)
class TS_CPP_API TEngine {
public:
    static TEngine& Instance();

    FUNCTION(CSHARP)
    void Init();
    FUNCTION(CSHARP)
    void Tick();

    FUNCTION(CSHARP)
    Render* IRender();

    FUNCTION(CSHARP)
    void UpdateSize(VkRect2D Size);

private:
    TEngine();

private:
    Render render_;
};
}  // namespace TSEngine
