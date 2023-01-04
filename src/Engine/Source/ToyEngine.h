#pragma once

#include "Base/Macro.h"
namespace toystation {
CLASS(SINGLETON)
class TS_CPP_API ToyEngine {
public:
    static ToyEngine& Instance();

    FUNCTION(CSHARP)
    void Init();
    FUNCTION(CSHARP)
    void Tick();

    // FUNCTION(CSHARP)
    // Render* IRender();

    // FUNCTION(CSHARP)
    // void UpdateSize(VkRect2D Size);

private:
    ToyEngine();

private:
    //Render render_;
};
}  // namespace toystation
