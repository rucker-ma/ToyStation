//
// Created by ma on 2023/4/3.
//
#include "RenderGlobalData.h"
#include "Vulkan/Images.h"

namespace toystation {
void RenderGlobalData::AddRenderObject(std::shared_ptr<TObject> obj) {
    int id = obj->GetID();
    std::shared_ptr<RenderObject> render_object =
        std::make_shared<RenderObject>(id);

    std::shared_ptr<MeshComponent> meshes = obj->GetComponent<MeshComponent>();
    std::shared_ptr<MaterialComponent> materials =
        obj->GetComponent<MaterialComponent>();

    VkCommandBuffer cmd = render_context->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    // TODO:添加实现细节
    for (auto& submesh : meshes->GetSubMesh()) {
        std::shared_ptr<RenderMesh> render_mesh = render_object->CreateMesh();
        if(!submesh->Position().empty()) {
            render_mesh->position_buffer =
                render_context->GetAllocator()->CreateBuffer(
                    cmd, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    submesh->Position());
        }
        if(!submesh->Normal().empty()) {
            render_mesh->normal_buffer =
                render_context->GetAllocator()->CreateBuffer(
                    cmd, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, submesh->Normal());
        }
        if(!submesh->Indices().empty()) {
            render_mesh->indices_buffer =
                render_context->GetAllocator()->CreateBuffer(
                    cmd, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, submesh->Indices());
        }
        if(!submesh->TexCoord().empty()) {
            render_mesh->texcoord_buffer =
                render_context->GetAllocator()->CreateBuffer(
                    cmd, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    submesh->TexCoord());
        }
        if(!submesh->Tangent().empty()){
            render_mesh->has_tangent = true;
            render_mesh->tangent_buffer =
                render_context->GetAllocator()->CreateBuffer(
                    cmd, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    submesh->Tangent());
        }else{
            LogWarn("mesh not tangent data");
            render_mesh->has_tangent = false;
            render_mesh->tangent_buffer =
                render_context->GetAllocator()->CreateBuffer(
                    cmd, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                    submesh->Position());
        }

        render_mesh->indices_size = submesh->IndicesInfo().nums;
        switch (submesh->IndicesInfo().type) {
            case BufferType::TYPE_UNSIGNED_SHORT:
                render_mesh->indices_type = VK_INDEX_TYPE_UINT16;
                break;
            case BufferType::TYPE_UNSIGNED_INT:
                render_mesh->indices_type = VK_INDEX_TYPE_UINT32;
                break;
            default:
                assert(0&&"other type for indices is not supported");
                break ;
        }
        render_mesh->material_index = submesh->MaterialIndex();
        render_mesh->UpdateSet();
    }

    for (auto& material : materials->Materials()) {
        std::shared_ptr<RenderMaterial> render_material =
            render_object->CreateMaterial();
        std::shared_ptr<Texture> basecolor = material->Basecolor();
        // TODO:确认image的格式，数据类型，usage等
        if (basecolor) {
            CreateRenderTexture(cmd, basecolor, render_material->basecolor);
        }
        std::shared_ptr<Texture> occlusion = material->OcclusionTexture();
        if (occlusion) {
            CreateRenderTexture(cmd, occlusion, render_material->occlusion);
        }
        std::shared_ptr<Texture> metallic =
            material->MetallicRoughnessTexture();
        if (metallic) {
            CreateRenderTexture(cmd, metallic,render_material->metallic_roughness);
            render_material->factor_buffer =  render_context->GetAllocator()->CreateBuffer(
                cmd, sizeof(material->Factor()),&material->Factor(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        }
        std::shared_ptr<Texture> normal = material->NormalTexture();
        if (normal) {
            CreateRenderTexture(cmd, normal, render_material->normal);
        }else{
            //mock normal map
            CreateRenderTexture(cmd, basecolor, render_material->normal);
        }
        render_material->UpdateSet();
    }
    render_context->GetCommandPool()->SubmitAndWait(cmd);
    render_resource->render_objects_.insert(std::make_pair(id, render_object));
}
void RenderGlobalData::CreateRenderTexture(
    VkCommandBuffer& cmd, std::shared_ptr<Texture> texture_data,
    RHITexture& texture) {
    VkImageCreateInfo create_info = MakeImage2DCreateInfo(
        {texture_data->width, texture_data->height}, VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    RHIImage rhi_image = render_context->GetAllocator()->CreateImage(
        cmd, texture_data->data.size(), texture_data->data.data(), create_info,
        VK_IMAGE_LAYOUT_GENERAL);
    VkImageViewCreateInfo color_view = MakeImage2DViewCreateInfo(
        rhi_image.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM);
    VkSamplerCreateInfo sampler_create_info{};
    texture = render_context->GetAllocator()->CreateTexture(
        rhi_image, color_view, sampler_create_info);
}
}  // namespace toystation