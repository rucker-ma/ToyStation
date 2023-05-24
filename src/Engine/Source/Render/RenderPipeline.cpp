#include "RenderPipeline.h"

#include "MainCameraPass.h"
#include "ConvertPass.h"
#include "TextPass.h"
#include "SkyboxPass.h"
#include "PostProcessPass.h"
//#include "RenderSystem.h"
#include "ToyEngine.h"

namespace toystation {
void RenderPipeline::Initialize() {
    main_camera_pass_ = std::make_shared<MainCameraPass>();
    text_pass_ = std::make_shared<TextPass>();
    skybox_pass_ = std::make_shared<SkyboxPass>();
    postprocess_pass_ = std::make_shared<PostProcessPass>();

    RenderPassInitInfo info = GetRenderPassInitInfo();
    main_camera_pass_->Initialize(info);
    skybox_pass_->Initialize(info);
    text_pass_->Initialize(info);
    postprocess_pass_->Initialize(info);
    update_shader_ = false;
}
void RenderPipeline::PreRender(){
    main_camera_pass_->PostInitialize();
    skybox_pass_->PostInitialize();
    text_pass_->PostInitialize();
}
void RenderPipeline::Tick() {
    if(update_shader_){
        UpdateShader();
        update_shader_ = false;
    }
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime).count();
    auto resource = RenderSystem::kRenderGlobalData.render_resource;
    resource->ubo_.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    //    ubo.model = glm::mat4(1.0F);
    resource->ubo_.model[3][0] = 2.0;
    resource->ubo_.model[3][1] = 0.0;
    resource->ubo_.model[3][2] = -0.3;

    auto controlller = kEngine.GetWorldManager().ActiveLevel()->GetController();
    auto camera_component = controlller->GetComponent<CameraComponent>();
    assert(camera_component);
    resource->ubo_.view = camera_component->GetView();
    resource->ubo_.proj = camera_component->GetProjection();
    resource->ubo_.camera_position = camera_component->GetPosition();

    main_camera_pass_->Draw();
    skybox_pass_->Draw();
    text_pass_->Draw();
    postprocess_pass_->Draw();
    //text_pass使用的是正交投影，会修改`resource->ubo_.proj`,因此放在最后

}
RenderPassInitInfo RenderPipeline::GetRenderPassInitInfo() {
    RenderPassInitInfo info;
    info.context = RenderSystem::kRenderGlobalData.render_context;
    info.resource = RenderSystem::kRenderGlobalData.render_resource;
    return info;
}

void RenderPipeline::UpdateShader(){
    main_camera_pass_->ResetPass();
    skybox_pass_->ResetPass();
    postprocess_pass_->ResetPass();
}
}  // namespace toystation