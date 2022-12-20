#include "TSEngine_c.h"
void TEngine_Init() { return TSEngine::TEngine::Instance().Init(); }
void TEngine_Tick() { return TSEngine::TEngine::Instance().Tick(); }
TSEngine::Render* TEngine_IRender() {
    return TSEngine::TEngine::Instance().IRender();
}
void TEngine_UpdateSize(VkRect2D Size) {
    return TSEngine::TEngine::Instance().UpdateSize(Size);
}
