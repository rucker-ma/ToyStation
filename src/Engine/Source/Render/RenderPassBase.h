#pragma once
#include "RenderContext.h"

namespace toystation {

class RenderAttachRef {
public:
    static constexpr uint32_t kRenderAttachRefColor = 0;
    static constexpr uint32_t kRenderAttachRefDepth = 1;
};

struct RenderPassInitInfo {
    std::shared_ptr<RenderContext> context;
};

class RenderPassBase {
public:
    struct RenderPipeline {
        VkPipeline pipeline;
        VkPipelineLayout layout;
    };
    struct Descriptor {
        VkDescriptorSetLayout layout;
        VkDescriptorSet descriptor_set;
    };

    virtual void Initialize(RenderPassInitInfo& info){};
    virtual void PostInitialize(){};
    virtual void PrepareData(){};
    virtual void Draw(){};
protected:
    VkRenderPass render_pass_;
    RenderPipeline pipeline_;
    Descriptor descriptor_;
};
}  // namespace toystation