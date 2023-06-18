//
// Created by ma on 2023/6/5.
//
#pragma once
#include "Base/Vector.h"
#include "RenderPassBase.h"
namespace toystation{

class DebugLayerPass : public RenderPassBase {
public:
    virtual ~DebugLayerPass(){};
    virtual void Initialize(RenderPassInitInfo& info) override;
    virtual void PostInitialize()override;
    virtual void Draw() override;
    //void ResetPass()override;
private:
    void UpdateVertex();
    void SetupRenderPass();
    void SetupDescriptorSetLayout();
    void SetupPipeline();
    void SetupFrameBuffer();
private:
    enum SubPass{
        SubPass_Default=0,
        SubPass_Count
    };
    RHIBuffer hit_vertex_buffer_;
    int hit_vertex_count_;
    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    std::vector<Vector3> outline_vertex_;
    int vertex_count_;
    RHIBuffer outline_buffer_;
};
}
