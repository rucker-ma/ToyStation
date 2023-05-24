//
// Created by ma on 2023/5/9.
//
#include "RenderContext.h"
#include "RenderPassBase.h"
#include "RenderResource.h"

namespace toystation{

class SkyboxPass:public RenderPassBase{
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
    void SetupFrameBuffer(RenderPassInitInfo& info);

private:
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
};

}
