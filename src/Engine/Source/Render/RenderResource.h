#pragma once

#include "Vulkan/DescriptorSets.h"
#include "Vulkan/ResourceAllocator.h"
#include "Framework/TObject.h"
#include "RenderObject.h"
#include "RenderContext.h"
namespace toystation {

#define GBUFFER_COUNT 2
//暂存需要更新的对象和标脏的组件
class RenderDataUpdatePipe{
public:
    void AddDirtyComponent(std::shared_ptr<TObject> obj,ComponentType type);
    const std::map<std::shared_ptr<TObject>,std::vector<ComponentType>>&
    DirtyComponents();
    void RenderClear();
private:
    std::map<std::shared_ptr<TObject>,std::vector<ComponentType>> swap_dirty_components0_;
    std::map<std::shared_ptr<TObject>,std::vector<ComponentType>> swap_dirty_components1_;
    bool use_first_;
};

class RenderResource {
public:
    RenderResource();
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

    std::shared_ptr<RenderDataUpdatePipe> update_pipe;
};


}  // namespace toystation