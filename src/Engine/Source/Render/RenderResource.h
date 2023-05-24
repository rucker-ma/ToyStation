#pragma once

#include "Vulkan/DescriptorSets.h"
#include "Vulkan/ResourceAllocator.h"
#include "RenderObject.h"
#include "RenderContext.h"
namespace toystation {

#define GBUFFER_COUNT 2

class RenderResource {
public:
    void Initialize(std::shared_ptr<RenderContext> context);
public:
    std::unordered_map<int,std::shared_ptr<RenderObject>> render_objects_;
    RHITexture skybox_texture;
    RHITexture irradiance_texture;
    RHITexture radiance_texture;
    RHITexture brdf_texture;
    std::vector<RHITexture> gbuffers;
    RHITexture shading_texture;
    RHITexture depth_texture;

    UniformBuffer ubo_;
    RHIBuffer ubo_buffer_;
};
}  // namespace toystation