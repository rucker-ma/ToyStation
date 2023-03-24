#pragma once

// #define GLM_FORCE_RADIANS
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>

#include "Base/Vector.h"

#include "RenderContext.h"
#include "RenderPassBase.h"
#include "RenderResource.h"

#include "Vulkan/DescriptorSets.h"

namespace toystation {

enum MainCameraSubpassType{
    SUBPASS_BASEPASS = 0,
    SUBPASS_YUV_TRANSFER = 1
};

VkShaderModule GetShader(std::string path, std::shared_ptr<VkContext> ctx);

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

    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;

    RHITexture image_tex_;
};
}  // namespace toystation
