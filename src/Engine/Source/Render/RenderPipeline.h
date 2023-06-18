#pragma once
#include <memory>

#include "RenderPass/RenderPassBase.h"

namespace toystation {

class RenderPipeline {
public:
    void Initialize();
    void PreRender();
    void Tick();
    void MarkUpdateShader(){ update_shader_ = true;}
    void PrepareData();
    RenderPassInitInfo GetRenderPassInitInfo();
private:
    void UpdateShader();
private:
    std::shared_ptr<RenderPassBase> main_camera_pass_;
    std::shared_ptr<RenderPassBase> text_pass_;
    std::shared_ptr<RenderPassBase> skybox_pass_;
    std::shared_ptr<RenderPassBase> postprocess_pass_;
    std::shared_ptr<RenderPassBase> debug_pass_;
    bool update_shader_;
};
}  // namespace toystation