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
    update_shader_ = false;
//    text_pass_->Initialize(info);
}
void RenderPipeline::Tick() {
    if(update_shader_){
        UpdateShader();
        update_shader_ = false;
    }
    main_camera_pass_->Draw();
}
RenderPassInitInfo RenderPipeline::GetRenderPassInitInfo() {
    RenderPassInitInfo info;
    info.context = RenderSystem::kRenderGlobalData.render_context;
    info.resource = RenderSystem::kRenderGlobalData.render_resource;
    return info;
}

void RenderPipeline::UpdateShader(){
    auto main_pass = std::dynamic_pointer_cast<MainCameraPass>( main_camera_pass_);
    if(main_pass) {
        main_pass->RebuildShaderAndPipeline();
    }
}

}  // namespace toystation