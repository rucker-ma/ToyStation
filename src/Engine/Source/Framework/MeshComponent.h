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
    Mesh_Indices
};

enum class MeshDrawMode{
    Draw_Triangle,
    Draw_Line,
    Draw_Point
};
class SubMesh{
    struct TextureBound{
        std::vector<unsigned char> texcoord;
        std::weak_ptr<Material> material;
    };
public:
    void SetDrawMode(MeshDrawMode mode);
    void AddData(MeshDataType type,std::vector<unsigned char>&data);
    void AddTexture(std::vector<unsigned char>&tex_coord,std::weak_ptr<Material>material);
    void SetLocalMatrix(Matrix4 mat);

private:
    std::vector<unsigned char> position_buffer_;
    std::vector<unsigned char> normal_buffer_;
    std::vector<unsigned char> tangent_buffer_;
    std::vector<TextureBound> tex_refs_;
    Matrix4 local_mat_;
    MeshDrawMode draw_mode_;
};

class MeshComponent:public TComponent{
public:
    ComponentType GetType()override;

    std::shared_ptr<SubMesh> CreateSubMesh();

private:
    std::vector<std::shared_ptr<SubMesh>> meshes_;
};
}