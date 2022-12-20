#include "TSEngine.h"

#include "Render/VulkanContext.h"
namespace TSEngine {
TEngine& TEngine::Instance() {
    static TEngine Engine;
    return Engine;
}

void TEngine::Init() { render_.Init(); }
void TEngine::Tick() { render_.Draw(); }
Render* TEngine::IRender() { return &render_; }
void TEngine::UpdateSize(VkRect2D Size) { render_.UpdateSize(Size); }
TEngine::TEngine() : render_() {}
}  // namespace TSEngine
