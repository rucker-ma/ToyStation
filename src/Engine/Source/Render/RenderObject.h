//
// Created by ma on 2023/3/23.
//
#pragma once
#include <memory>
#include "Base/Vector.h"

#include "Vulkan/ResourceAllocator.h"
#include "Vulkan/DescriptorSets.h"

namespace toystation{

struct UniformBuffer {
    Matrix4 model;
    Matrix4 view;
    Matrix4 proj;
    bool has_tangent;
};
class RenderMesh{
public:
    RHIBuffer position_buffer;
    RHIBuffer normal_buffer;
    RHIBuffer indices_buffer;
    RHIBuffer texcoord_buffer;
    RHIBuffer tangent_buffer;
    RHIBuffer uniform_buffer;

    VkIndexType indices_type;
    uint32_t indices_size;
    int material_index;
    DescriptorSetContainer set_container;
    bool has_tangent;
    void UpdateSet();
    void UpdateUniform(UniformBuffer& buffer);
};

class RenderMaterial{
public:
    RenderMaterial();
    RHITexture basecolor;
    RHITexture occlusion;
    RHITexture metallic_roughness;
    RHITexture normal;
    RHIBuffer factor_buffer;
    DescriptorSetContainer set_container;
    void UpdateSet();
    bool IsValid(){return valid_;}
private:
    bool valid_;
};

class RenderObject
{
public:
    RenderObject(int id);
    int GetID()const{return id_;}
    std::vector<std::shared_ptr<RenderMesh>>& Meshes(){return meshes_;}
    std::shared_ptr<RenderMesh> CreateMesh();
    std::shared_ptr<RenderMaterial> CreateMaterial();
    std::shared_ptr<RenderMaterial> Material(int index){return materials_[index];}
private:
    int id_;
    std::vector<std::shared_ptr<RenderMesh>> meshes_;
    std::vector<std::shared_ptr<RenderMaterial>> materials_;
};
}