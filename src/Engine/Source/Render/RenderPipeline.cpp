#include "RenderPipeline.h"

#include "MainCameraPass.h"
#include "RenderSystem.h"
namespace toystation {
void RenderPipeline::Initialize() {
    main_camera_pass_ = std::make_shared<MainCameraPass>();

    RenderPassInitInfo info;
    info.context = RenderSystem::kRenderGlobalData.render_context;

    main_camera_pass_->Initialize(info);
}
void RenderPipeline::Tick() {
    main_camera_pass_->Draw();
}
}  // namespace toystation