//
// Created by ma on 2023/5/18.
//

#pragma once
#include "RenderContext.h"
#include "RenderPassBase.h"
#include "RenderResource.h"
#include "GizmoModel.h"

namespace toystation{
class PostProcessPass:public RenderPassBase {
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
    void SetupRenderPass();
    void SetupDescriptorSetLayout();
    void SetupPipeline();
    void SetupFrameBuffer();
    struct PostProcessUniform{
        Vector2 size;
        Vector3 color;
    };
private:
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    RHIBuffer frag_uniform_;
    //std::shared_ptr<MoveGizmoModel> move_model_;
    std::shared_ptr<RenderObject> move_coord_object_;
};
}