#pragma once
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

    DescriptorSetContainer set_container_;
    std::shared_ptr<RenderContext> context_;
    std::vector<VkFramebuffer> framebuffers_;

};
}  // namespace toystation
