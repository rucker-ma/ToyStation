#include "ToyEngine.h"

namespace toystation {
ToyEngine& ToyEngine::Instance() {
    static ToyEngine engine;
    return engine;
}

void ToyEngine::Init() {  // render_.Init();
}
void ToyEngine::Tick() {  // render_.Draw();
}
// Render* TEngine::IRender() { return &render_; }
// void TEngine::UpdateSize(VkRect2D Size) { render_.UpdateSize(Size); }
ToyEngine::ToyEngine() {}
}  // namespace toystation
