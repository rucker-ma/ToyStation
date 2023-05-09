//
// Created by ma on 2023/3/23.
//
#include "RenderObject.h"

#include <cassert>

#include "RenderSystem.h"

namespace toystation {

void RenderMesh::UpdateSet() {
    set_container.Init(
        RenderSystem::kRenderGlobalData.render_context->GetContext().get());
    set_container.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                             VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT);

    uniform_buffer =
        RenderSystem::kRenderGlobalData.render_context->GetAllocator()
            ->CreateBuffer(sizeof(toystation::UniformBuffer),
                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = uniform_buffer.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);

    set_container.InitLayout();
    set_container.InitPool(1);
    std::vector<VkWriteDescriptorSet> sets;

    sets.push_back(
        set_container.MakeWrite(0, 0, &buffer_info /* uniform buffer*/));
    set_container.UpdateSets(sets);
}
void RenderMesh::UpdateUniform(UniformBuffer& buffer)
{
    UniformBuffer local_buffer = buffer;
    local_buffer.has_tangent = has_tangent;
    local_buffer.model = glm::rotate(buffer.model,(float)90.0,Vector3(0.0,0.0,1.0));
    void*data = RenderSystem::kRenderGlobalData.render_context->GetAllocator()->Map(uniform_buffer);
    memcpy(data,&local_buffer,sizeof(local_buffer));
    RenderSystem::kRenderGlobalData.render_context->GetAllocator()->UnMap(uniform_buffer);
}
RenderMaterial::RenderMaterial():valid_(false){}
void RenderMaterial::UpdateSet() {
    set_container.Init(
        RenderSystem::kRenderGlobalData.render_context->GetContext().get());
    set_container.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                             VK_SHADER_STAGE_FRAGMENT_BIT);
    set_container.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                             VK_SHADER_STAGE_FRAGMENT_BIT);
    set_container.AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                             VK_SHADER_STAGE_FRAGMENT_BIT);
    set_container.AddBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                             VK_SHADER_STAGE_FRAGMENT_BIT);
    set_container.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
                             VK_SHADER_STAGE_FRAGMENT_BIT,1);

    set_container.InitLayout();
    set_container.InitPool(2);
    std::vector<VkWriteDescriptorSet> sets;
    if (basecolor.image) {
        sets.push_back(set_container.MakeWrite(0, 0, &basecolor.descriptor));
    }
    if (occlusion.image) {
        sets.push_back(set_container.MakeWrite(0, 1, &occlusion.descriptor));
    }
    if(metallic_roughness.image){
        sets.push_back(set_container.MakeWrite(0, 2, &metallic_roughness.descriptor));
    }
    if(normal.image){
        sets.push_back(set_container.MakeWrite(0, 3, &normal.descriptor));
    }
    if(!factor_buffer.buffer) {
        factor_buffer =
            RenderSystem::kRenderGlobalData.render_context->GetAllocator()
                ->CreateBuffer(sizeof(toystation::Material::MaterialFactor),
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = factor_buffer.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::Material::MaterialFactor);
    sets.push_back(
        set_container.MakeWrite(1, 0, &buffer_info /* uniform buffer*/));
    if(sets.empty()){
        return;
    }
    set_container.UpdateSets(sets);
    valid_ = true;
}

RenderObject::RenderObject(int id) {
    assert(id > 0);
    id_ = id;
}
std::shared_ptr<RenderMesh> RenderObject::CreateMesh() {
    std::shared_ptr<RenderMesh> mesh = std::make_shared<RenderMesh>();
    meshes_.push_back(mesh);
    return mesh;
}
std::shared_ptr<RenderMaterial> RenderObject::CreateMaterial() {
    std::shared_ptr<RenderMaterial> material =
        std::make_shared<RenderMaterial>();
    materials_.push_back(material);
    return material;
}
}  // namespace toystation