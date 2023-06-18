//
// Created by ma on 2023/5/18.
//
#include "GizmoModel.h"
#define _USE_MATH_DEFINES
#include <math.h>

namespace toystation{


//              _______
//         (1) |      |(0)
//            |      |  |
//        (2  |______|(3)|
//           |       |  |(4)
//       (6) |_______|/ (7)
//
std::vector<Vector3> GetBoxVertex(float edge_length){
    float length = edge_length/2;
    return {
        {length,length,length},{-length,length,length},{-length,-length,length},{length,-length,length},
        {length,length,-length},{-length,length,-length},{-length,-length,-length},{length,-length,-length}
    };
    //return res;
}
std::vector<IVector3> GetBoxIndice(){
    return std::vector<IVector3> {
        {0,1,2},{0,2,3},
        {0,3,4},{3,4,7},
        {0,1,5},{0,5,4},
        {1,2,5},{2,5,6},
        {2,3,6},{3,6,7},
        {4,5,6},{4,6,7}
    };
}


MoveGizmoModel::MoveGizmoModel(){
    axis_length_ = 2*10;
    axis_radius_ = 0.1*10;
    arrow_radius_ = 0.3*10;
    arrow_length_ = 0.5*10;
    samples_ = 60*2;
}
MoveGizmoModel::~MoveGizmoModel(){
}
void MoveGizmoModel::GenerateModel(){
    Matrix4 mat = Matrix4(1.0);
    auto material_comp = CreateComponent<MaterialComponent>();
    auto mesh_comp = CreateComponent<MeshComponent>();

    Generate(mat);
    auto material = material_comp->CreateMaterial();
    material->Factor().base_color = Vector4 (1.0,0,0,1.0);
    GetComponent<MeshComponent>()->GetSubMesh().back()->SetMaterialIndex(0);

    mat =glm::rotate(Matrix4(1.0),(float)M_PI_2,Vector3(0,1,0));
    Generate(mat);
    material = material_comp->CreateMaterial();
    material->Factor().base_color = Vector4 (0,1.0,0,1.0);
    GetComponent<MeshComponent>()->GetSubMesh().back()->SetMaterialIndex(1);

    mat =glm::rotate(Matrix4(1.0),(float)M_PI_2,Vector3(0,0,1));
    Generate(mat);
    material = material_comp->CreateMaterial();
    material->Factor().base_color = Vector4 (0,0,1.0,1.0);
    GetComponent<MeshComponent>()->GetSubMesh().back()->SetMaterialIndex(2);

    AddOrigin();
    material = material_comp->CreateMaterial();
    material->Factor().base_color = Vector4 (1.0,1.0,1.0,1.0);
    GetComponent<MeshComponent>()->GetSubMesh().back()->SetMaterialIndex(3);
}
std::vector<std::shared_ptr<SubMesh>> MoveGizmoModel::GetMeshes(){
    return GetComponent<MeshComponent>()->GetSubMesh();
}
void MoveGizmoModel::Generate(Matrix3 rotate){
    float min_radian = 2*M_PI/samples_;
    std::vector<Vector3> vertex;
    vertex.resize(samples_*3+2);

    int center = samples_*3;
    int cone = samples_*3+1;
    vertex[center] = rotate* Vector3{axis_length_,0,0};
    vertex[cone] = rotate* Vector3{arrow_length_+axis_length_,0,0};
    std::vector<IVector3> indices;

    for (int i = 0; i < samples_; ++i) {
        float radian = i*min_radian;
        float y = std::cos(radian);//*axis_radius_; //up direction
        float z = std::sin(radian);//*axis_radius_; // screen in direction
        //float x = axis_length_; //right
        vertex[i] = rotate* Vector3{axis_length_,y*axis_radius_,z*axis_radius_};
        vertex[i+samples_] = rotate* Vector3{0,y*axis_radius_,z*axis_radius_};
        vertex[i+samples_*2] =  rotate* Vector3{axis_length_,y*arrow_radius_,z*arrow_radius_};
        if(i != 0){
            indices.push_back({i-1,i,i-1+samples_});
            indices.push_back({i-1+samples_,i+samples_,i});
            indices.push_back({i-1+samples_*2,i+samples_*2,center});
            indices.push_back({i-1+samples_*2,i+samples_*2,cone});
        }else{
            indices.push_back({samples_-1,i,samples_-1 + samples_});
            indices.push_back({samples_-1+samples_,i+samples_,i});
            indices.push_back({samples_-1+samples_*2,i+samples_*2,center});
            indices.push_back({samples_-1+samples_*2,i+samples_*2,cone});
        }
    }
    auto mesh_comp = GetComponent<MeshComponent>();
    auto submesh =mesh_comp->CreateSubMesh();
    submesh->AddData(MeshDataType::Mesh_Position,(unsigned char*)vertex.data(),
                     sizeof(Vector3)*vertex.size());
    submesh->AddData(MeshDataType::Mesh_Indices,(unsigned char*)indices.data(),
                     sizeof(IVector3)*indices.size());
    submesh->IndicesInfo().nums = indices.size()*3;
    mesh_comp->AddSubMesh(submesh);
    //meshes_.push_back(submesh);
}
void MoveGizmoModel::AddOrigin(){
    float half_box_length = axis_radius_*1.3;
    auto mesh_comp = GetComponent<MeshComponent>();
    auto submesh = mesh_comp->CreateSubMesh();
    auto vertex = GetBoxVertex(half_box_length);
    auto indices = GetBoxIndice();
    submesh->AddData(MeshDataType::Mesh_Position,(unsigned char*)vertex.data(),
                     sizeof(Vector3)*vertex.size());
    submesh->AddData(MeshDataType::Mesh_Indices,(unsigned char*)indices.data(),
                     sizeof(IVector3)*indices.size());
    submesh->IndicesInfo().nums = indices.size()*3;
    mesh_comp->AddSubMesh(submesh);
//    meshes_.push_back(submesh);
}

RotateGizmoModel::RotateGizmoModel(){
    large_radius_ = 2.5;
    small_radius_ = 2.3;
    samples_ = 60;
}
RotateGizmoModel::~RotateGizmoModel(){
    meshes_.clear();
}
void RotateGizmoModel::GenerateModel(){
    Matrix4 mat = Matrix4(1.0);
    Generate(mat);
    mat =glm::rotate(Matrix4(1.0),(float)M_PI_2,Vector3(0,1,0));
    Generate(mat);
    mat =glm::rotate(Matrix4(1.0),(float)-M_PI_2,Vector3(0,0,1));
    Generate(mat);
}
std::vector<std::shared_ptr<SubMesh>> RotateGizmoModel::GetMeshes(){
    return meshes_;
}
void RotateGizmoModel::Generate(Matrix3 rotate){
    float min_radian = M_PI_2/samples_;
    std::vector<Vector3> vertex;
    vertex.resize(samples_*2);
    vertex.push_back(Vector3(0.0));

    std::vector<IVector3> indices;

    for (int i = 0; i < samples_; ++i) {
        float radian = i*min_radian;
        float y = std::cos(radian);//*axis_radius_; //up direction
        float z = std::sin(radian);//*axis_radius_; // screen in direction

        vertex[i] = rotate*Vector3 (0,y*small_radius_,z*small_radius_);
        vertex[i+samples_] = rotate*Vector3 (0,y*large_radius_,z*large_radius_);
        //TODO:考虑加上内扇面并使用其它颜色渲染
        if(i!=0){
            indices.push_back({i-1,i,i-1+samples_});
            indices.push_back({i,i-1+samples_,i+samples_});
        }
    }
    auto mesh_comp = CreateComponent<MeshComponent>();
    auto submesh =mesh_comp->CreateSubMesh();

    submesh->AddData(MeshDataType::Mesh_Position,(unsigned char*)vertex.data(),
                     sizeof(Vector3)*vertex.size());
    submesh->AddData(MeshDataType::Mesh_Indices,(unsigned char*)indices.data(),
                     sizeof(IVector3)*indices.size());

    submesh->IndicesInfo().nums = indices.size()*3;
    meshes_.push_back(submesh);
}
}