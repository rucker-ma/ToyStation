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
            position_buffer_.buffer = std::move(data);
            break ;
        case MeshDataType::Mesh_Normal:
            normal_buffer_.buffer = std::move(data);
            break ;
        case MeshDataType::Mesh_Tangent:
            tangent_buffer_.buffer = std::move(data);
            break ;
        case MeshDataType::Mesh_Coord:
            texcoord_.buffer = std::move(data);
            break ;
        case MeshDataType::Mesh_Indices:
            indices_buffer_.buffer = std::move(data);
            break ;
    }
}
void SubMesh::AddData(MeshDataType type,VertexDataInfo& data)
{
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
        case MeshDataType::Mesh_Coord:
            texcoord_ = std::move(data);
            break ;
        case MeshDataType::Mesh_Indices:
            indices_buffer_ = std::move(data);
            break ;
    }
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
std::vector<std::shared_ptr<SubMesh>>& MeshComponent::GetSubMesh() {
    return meshes_;
}
void MeshComponent::SetLocalMatrix(Matrix4 mat) {
    for(auto& mesh:meshes_){
        mesh->SetLocalMatrix(mat);
    }
}

}