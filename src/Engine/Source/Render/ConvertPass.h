#pragma once
#include "RenderPassBase.h"

namespace toystation {
class ConvertPass : public RenderPassBase {
public:
    virtual ~ConvertPass() = default;
    virtual void Initialize(RenderPassInitInfo& info) override;
    virtual void Draw() override;

private:
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupTexture(RenderPassInitInfo& info);
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
};
}  // namespace toystation