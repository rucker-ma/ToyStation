//
// Created by ma on 2023/3/23.
//
#pragma once
#include <memory>
#include "Base/Vector.h"

#include "Vulkan/ResourceAllocator.h"
#include "Vulkan/DescriptorSets.h"
#include "Framework/Component/MaterialComponent.h"
namespace toystation{

struct UniformBuffer {
    Matrix4 model;
    Matrix4 view;
    Matrix4 proj;
    Vector3 camera_position ;
    Vector3 light_color;
    int has_tangent;
    int has_envmap;
    UniformBuffer(){
        has_tangent = false;
        light_color = Vector3(1.0);
    }
};
class RenderMesh{
public:
    RenderMesh();
    RHIBuffer position_buffer;
    RHIBuffer normal_buffer;
    RHIBuffer indices_buffer;
    RHIBuffer texcoord_buffer;
    RHIBuffer tangent_buffer;
//    RHIBuffer uniform_buffer;

    VkIndexType indices_type;
    uint32_t indices_size;
    int material_index;
    bool has_tangent;
    Matrix4 GetModel();
private:
    Matrix4 model_{1.0};
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

class RenderObject{
public:
    RenderObject() =default;
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