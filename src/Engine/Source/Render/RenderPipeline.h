#pragma once
#include <memory>

#include "RenderPassBase.h"

namespace toystation {

class RenderPipeline {
public:
    void Initialize();
    void Tick();
private:
    std::shared_ptr<RenderPassBase> main_camera_pass_;
};
}  // namespace toystation