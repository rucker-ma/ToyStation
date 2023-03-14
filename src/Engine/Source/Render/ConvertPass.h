#pragma once

#include <memory>

#include "RenderPassBase.h"
namespace toystation {
class FrameConvertPass : public RenderPassBase {
public:
    //TODO:修改此处的裸指针传递
    virtual std::unique_ptr<RenderFrame> GetConvertFrame()=0;
};

class FrameConvertYCrCbPass : public FrameConvertPass {
public:
    virtual ~FrameConvertYCrCbPass() = default;
    void Initialize(RenderPassInitInfo& info) override;
    void Draw() override;
    //TODO:修改为unique_ptr
    //指针由RenderSystem中DataMsg<RenderFrame>使用和释放
    std::unique_ptr<RenderFrame> GetConvertFrame()override;
private:
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupTexture(RenderPassInitInfo& info);

private:
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    struct ConvertYCrCbResource{
        DescriptorSetContainer set_container;
        RHIImage comp_y;
        Texture tex_y;
        RHIImage comp_cb;
        Texture tex_cb;
        RHIImage comp_cr;
        Texture tex_cr;
        std::vector<VkFramebuffer> framebuffers;
    };
    ConvertYCrCbResource convert_resource_;
};

class FrameConvertNV12Pass:public FrameConvertPass{
public:
    virtual ~FrameConvertNV12Pass() = default;
    void Initialize(RenderPassInitInfo& info) override;
    void Draw() override;
    //TODO:修改为unique_ptr
    //指针由RenderSystem中DataMsg<RenderFrame>使用和释放
    std::unique_ptr<RenderFrame> GetConvertFrame()override;
private:
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupTexture(RenderPassInitInfo& info);

private:
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    struct ConvertNV12Resource{
        DescriptorSetContainer set_container;
        RHIImage comp_y;
        Texture tex_y;
        RHIImage comp_uv;
        Texture tex_uv;
        std::vector<VkFramebuffer> framebuffers;
    };
    ConvertNV12Resource convert_resource_;
};
}  // namespace toystation