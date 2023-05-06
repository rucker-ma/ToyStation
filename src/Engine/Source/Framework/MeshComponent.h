//
// Created by ma on 2023/3/18.
//

#pragma once
#include <vector>
#include "TComponent.h"
#include "MaterialComponent.h"
#include "Base/Vector.h"

#include "Vulkan/ResourceAllocator.h"
namespace toystation{

enum class MeshDataType{
    Mesh_Normal,
    Mesh_Tangent,
    Mesh_Position,
    Mesh_Indices,
    Mesh_Coord
};

enum class MeshDrawMode{
    Draw_Triangle,
    Draw_Line,
    Draw_Point
};


class SubMesh{
public:
    void SetDrawMode(MeshDrawMode mode);
    void AddData(MeshDataType type,std::vector<unsigned char>&data);
    void AddData(MeshDataType type,VertexDataInfo& data);
    void SetMaterialIndex(int index){ material_index_ = index;}
    int MaterialIndex(){return material_index_;}
    std::vector<unsigned char>& Position(){return position_buffer_.buffer;}
    std::vector<unsigned char>& Normal(){return normal_buffer_.buffer;}
    std::vector<unsigned char>& Tangent(){return tangent_buffer_.buffer;}
    std::vector<unsigned char>& Indices(){return indices_buffer_.buffer;}
    VertexDataInfo& IndicesInfo(){return indices_buffer_;}
    std::vector<unsigned char>& TexCoord(){return texcoord_.buffer;}
    void SetLocalMatrix(Matrix4 mat);

private:
    VertexDataInfo position_buffer_;
    VertexDataInfo normal_buffer_;
    VertexDataInfo tangent_buffer_;
    VertexDataInfo indices_buffer_;
    VertexDataInfo texcoord_;
    int material_index_;
    Matrix4 local_mat_;
    MeshDrawMode draw_mode_;
};

class MeshComponent:public TComponent{
public:
    const static ComponentType Type = ComponentType::Component_Mesh;
    ComponentType GetType()override;
    void SetLocalMatrix(Matrix4 mat);
    std::shared_ptr<SubMesh> CreateSubMesh();
    std::vector<std::shared_ptr<SubMesh>>& GetSubMesh();
private:
    std::vector<std::shared_ptr<SubMesh>> meshes_;

};
}