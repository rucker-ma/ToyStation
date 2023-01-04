#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderPassBase.h"
#include "Vulkan/DescriptorSets.h"
#include "RenderContext.h"



namespace toystation {
class MainCameraPass : public RenderPassBase {
public:
    void Initialize(RenderPassInitInfo& info) override;
    void Draw() override;

private:
    void SetAttachmentResource();
    void SetupRenderPass(RenderPassInitInfo& info);
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupFrameBuffer(RenderPassInitInfo& info);

    void LoadTexture();
    void UpdateUniform();
    void SaveImage();
    DescriptorSetContainer set_container_;
    std::shared_ptr<RenderContext> context_;
    std::vector<VkFramebuffer> framebuffers_;
    Texture sampler_tex_;
    RHIImage color_image_;
};
}  // namespace toystation
