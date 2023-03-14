#include "RenderPipeline.h"

#include "MainCameraPass.h"
#include "ConvertPass.h"
#include "TextPass.h"
#include "RenderSystem.h"
namespace toystation {
void RenderPipeline::Initialize() {
    main_camera_pass_ = std::make_shared<MainCameraPass>();
    text_pass_ = std::make_shared<TextPass>();

    RenderPassInitInfo info = GetRenderPassInitInfo();
    main_camera_pass_->Initialize(info);
//    text_pass_->Initialize(info);
}
void RenderPipeline::Tick() {
    main_camera_pass_->Draw();
}
RenderPassInitInfo RenderPipeline::GetRenderPassInitInfo() {
    RenderPassInitInfo info;
    info.context = RenderSystem::kRenderGlobalData.render_context;
    info.resource = RenderSystem::kRenderGlobalData.render_resource;
    return info;
}
}  // namespace toystation