#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderContext.h"
#include "RenderPassBase.h"
#include "Vulkan/DescriptorSets.h"

namespace toystation {

class RenderFrameImpl : public RenderFrame {
public:
    RenderFrameImpl() = delete;
    RenderFrameImpl(const RenderFrameImpl& frame) = delete;
    RenderFrameImpl& operator=(const RenderFrameImpl& frame) = delete;

    RenderFrameImpl(std::shared_ptr<RenderContext> context, const RHIImage& img);
    virtual ~RenderFrameImpl();
    virtual unsigned char* Data()const;
    virtual unsigned int Width()const;
    virtual unsigned int Height()const;
    virtual RenderFrameType Type();

private:
    Buffer buf_;
    std::shared_ptr<RenderContext> context_;
    unsigned int width_;
    unsigned int height_;
    unsigned char* data_;
};

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
