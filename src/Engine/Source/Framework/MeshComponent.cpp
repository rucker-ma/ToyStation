//
// Created by ma on 2023/3/18.
//
#include "MeshComponent.h"
#include "Render/RenderSystem.h"

namespace toystation{

void SubMesh::SetDrawMode(MeshDrawMode mode) {
    draw_mode_ = mode;
}
void SubMesh::AddData(MeshDataType type, std::vector<unsigned char>& data) {
    switch (type) {
        case MeshDataType::Mesh_Position:
            position_buffer_ = std::move(data);
            break ;
        case MeshDataType::Mesh_Normal:
            normal_buffer_ = std::move(data);
            break ;
        case MeshDataType::Mesh_Tangent:
            tangent_buffer_ = std::move(data);
            break ;
    }
}
void SubMesh::AddTexture(std::vector<unsigned char>& tex_coord,
                         std::weak_ptr<Material> material) {
    auto rhi = RenderSystem::kRenderGlobalData.render_context;
    TextureBound bound;
    bound.texcoord = std::move(tex_coord);
    bound.material = material;
    tex_refs_.push_back(bound);
}
void SubMesh::SetLocalMatrix(Matrix4 mat) {
    local_mat_ =mat;
}

ComponentType MeshComponent::GetType() { return Component_Mesh; }
std::shared_ptr<SubMesh> MeshComponent::CreateSubMesh() {
   auto mesh = std::make_shared<SubMesh>();
    meshes_.push_back(mesh);
    return mesh;
}


}