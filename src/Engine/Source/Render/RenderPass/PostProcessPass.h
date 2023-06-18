//
// Created by ma on 2023/5/18.
//

#pragma once
#include "RenderPassBase.h"
#include "Render/GizmoModel.h"

namespace toystation{
class PostProcessPass:public RenderPassBase {
public:
    void Initialize(RenderPassInitInfo& info) override;
    void PostInitialize()override;
    void Draw() override;
    void ResetPass()override;
private:
    bool UpdateUniform();
    void UpdateFragUniform(Vector4 color);
    enum SubPass{
        SubPass_Default=0,
        SubPass_Count
    };
    void SetupRenderPass();
    void SetupDescriptorSetLayout();
    void SetupPipeline();
    void SetupFrameBuffer();
    struct PostProcessUniform{
        Vector4 color;
        Vector2 size;
    };
    struct UniformMaterail{
        DescriptorSetContainer set_container;
        RHIBuffer frag_buffer;
    };
private:
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    std::shared_ptr<GizmoModel> axis_model_;
    RHIBuffer local_ubo_buffer_;
    UniformBuffer uniform_buffer_;
    std::shared_ptr<RenderObject> move_coord_object_;
    std::vector<UniformMaterail> materials_;
};
}