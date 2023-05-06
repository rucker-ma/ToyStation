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

//now for debug
#define GBUFFER_COUNT 2

enum MainCameraSubpassType{
    SUBPASS_BASEPASS = 0,
    SUBPASS_YUV_TRANSFER = 1
};

//VkShaderModule GetShader(std::string path, std::shared_ptr<VkContext> ctx);

class MainCameraPass : public RenderPassBase {
    struct GBuffer{
        RHIImage image;
        RHITexture texture;
    };
public:
    void Initialize(RenderPassInitInfo& info) override;
    void Draw() override;
    void RebuildShaderAndPipeline();
private:

    void SetupRenderPass(RenderPassInitInfo& info);
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupFrameBuffer(RenderPassInitInfo& info);

    void UpdateUniform();
    void SaveImage();

    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    std::vector<GBuffer> gbuffers_;
    RHITexture image_tex_;
};
}  // namespace toystation
