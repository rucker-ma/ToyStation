#pragma once
#include <memory>

#include "RenderPassBase.h"

namespace toystation {

class RenderPipeline {
public:
    void Initialize();
    void Tick();
    RenderPassInitInfo GetRenderPassInitInfo();
private:
    std::shared_ptr<RenderPassBase> main_camera_pass_;
    std::shared_ptr<RenderPassBase> text_pass_;
};
}  // namespace toystation