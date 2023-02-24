#include "RenderPipeline.h"

#include "MainCameraPass.h"
#include "ConvertPass.h"

#include "RenderSystem.h"
namespace toystation {
void RenderPipeline::Initialize() {
    main_camera_pass_ = std::make_shared<MainCameraPass>();
    convert_pass_ = std::make_shared<ConvertPass>();
    RenderPassInitInfo info;
    info.context = RenderSystem::kRenderGlobalData.render_context;
    info.resource = RenderSystem::kRenderGlobalData.render_resource;

    main_camera_pass_->Initialize(info);
    convert_pass_->Initialize(info);
}
void RenderPipeline::Tick() {
    main_camera_pass_->Draw();
    convert_pass_->Draw();

}
}  // namespace toystation