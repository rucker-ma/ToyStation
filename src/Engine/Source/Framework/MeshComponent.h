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
class Ray{
public:
    Vector3 origin;
    Vector3 direction;
};
//class BoundingBox{
//public:
//    virtual ~BoundingBox()=default;
//    virtual bool Intersect(Ray ray) =0;
//    virtual bool Intersect(BoundingBox box)=0;
//    virtual BoundingBox Combine(BoundingBox box) =0;
//    virtual bool Inside(Vector3 point)=0;
//};

class AABBBox{
public:
    AABBBox(float xmin,float xmax,float ymin,float ymax,
            float zmin,float zmax);
    bool Intersect(Ray ray);
    bool Intersect(std::shared_ptr<AABBBox> box);
    std::shared_ptr<AABBBox> Combine(std::shared_ptr<AABBBox> box);
    std::shared_ptr<AABBBox> Copy();
    bool Inside(Vector3 point);
    float Xmin(){return xmin_;}
    float Xmax(){return xmax_;}
    float Ymin(){return ymin_;}
    float Ymax(){return ymax_;}
    float Zmin(){return zmin_;}
    float Zmax(){return zmax_;}
private:
    float xmin_;
    float xmax_;
    float ymin_;
    float ymax_;
    float zmin_;
    float zmax_;
    std::vector<Vector3> vertexs_;
};
class SubMesh{
public:
    //void SetDrawMode(MeshDrawMode mode);
    //std::shared_ptr<AABBBox>
    void AddData(MeshDataType type,std::vector<unsigned char>&data);
    void AddData(MeshDataType type,VertexDataInfo& data);
    void AddData(MeshDataType type,unsigned char* data,size_t length);
    void SetMaterialIndex(int index){ material_index_ = index;}
    int MaterialIndex(){return material_index_;}
    std::vector<unsigned char>& Position(){return position_buffer_.buffer;}
    std::vector<unsigned char>& Normal(){return normal_buffer_.buffer;}
    std::vector<unsigned char>& Tangent(){return tangent_buffer_.buffer;}
    std::vector<unsigned char>& Indices(){return indices_buffer_.buffer;}
    VertexDataInfo& IndicesInfo(){return indices_buffer_;}
    std::vector<unsigned char>& TexCoord(){return texcoord_.buffer;}
    void SetLocalMatrix(Matrix4 mat);
    std::shared_ptr<AABBBox> BoundingBox(){return bounding_box_;}
private:
    void GenerateBoundingBox();
private:
    VertexDataInfo position_buffer_;
    VertexDataInfo normal_buffer_;
    VertexDataInfo tangent_buffer_;
    VertexDataInfo indices_buffer_;
    VertexDataInfo texcoord_;
    int material_index_;
    Matrix4 local_mat_;
    MeshDrawMode draw_mode_;
    std::shared_ptr<AABBBox> bounding_box_;
};

class MeshComponent:public TComponent{
public:
    const static ComponentType Type = ComponentType::Component_Mesh;
    ComponentType GetType()override;
    void SetLocalMatrix(Matrix4 mat);
//    std::shared_ptr<SubMesh> CreateSubMesh();
    void AddSubMesh(std::shared_ptr<SubMesh> mesh);
    std::vector<std::shared_ptr<SubMesh>>& GetSubMesh();
    std::shared_ptr<AABBBox> BoundingBox(){return bounding_box_;}
private:
    std::vector<std::shared_ptr<SubMesh>> meshes_;
    std::shared_ptr<AABBBox> bounding_box_;

};
}