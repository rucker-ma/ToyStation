#pragma once

#include "Base/Vector.h"

#include "RenderPassBase.h"
#include "Vulkan/DescriptorSets.h"

namespace toystation {

//now for debug

class MainCameraPass : public RenderPassBase {
public:
    void Initialize(RenderPassInitInfo& info) override;
    void PostInitialize()override;
    void Draw() override;
    void ResetPass()override;
private:
    enum SubPass{
        SubPass_Default=0,
        SubPass_Count
    };
    void SetupRenderPass(RenderPassInitInfo& info);
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupSkyboxPipeline(RenderPassInitInfo& info);
    void SetupFrameBuffer(RenderPassInitInfo& info);

    void UpdateUniform(std::shared_ptr<RenderMesh> mesh);
    void SaveImage();
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;

    RHIBuffer uniform_buffer_;
};
}  // namespace toystation
