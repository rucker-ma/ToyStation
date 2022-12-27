#pragma once
#include "RenderPipeline.h"
#include "Vulkan/VkContext.h"

namespace toystation {
class RenderSystem {
public:
    void Initialize();
    void Tick();

private:
    std::shared_ptr<RenderPipeline> render_pipeline_;
};
}  // namespace toystation