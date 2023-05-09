#pragma once
#include "RenderContext.h"
#include "RenderResource.h"

namespace toystation {

class RenderAttachRef {
public:
    static constexpr uint32_t kRenderAttachRefColor = 0;
    static constexpr uint32_t kRenderAttachRefDepth = 1;
    static constexpr uint32_t kGbuffer0 = 2;
    static constexpr uint32_t kGbuffer1 = 3;
};

struct RenderPassInitInfo {
    std::shared_ptr<RenderContext> context;
    std::shared_ptr<RenderResource> resource;
};

class RenderPassBase {
public:
    struct RenderPipeline {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };
    struct Descriptor {
        std::vector<VkDescriptorSetLayout> layout;
        VkDescriptorSet descriptor_set;
    };

    virtual void Initialize(RenderPassInitInfo& info){};
    virtual void PostInitialize(){};
    virtual void PrepareData(){};
    virtual void Draw()=0;
protected:
    VkRenderPass render_pass_;
    std::vector<RenderPipeline> pipelines_;
    Descriptor descriptor_;
};
}  // namespace toystation