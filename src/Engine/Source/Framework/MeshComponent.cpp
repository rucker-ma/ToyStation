//
// Created by ma on 2023/3/18.
//
#include "MeshComponent.h"
#include "Render/RenderSystem.h"

namespace toystation{

AABBBox::AABBBox(float xmin,float xmax,float ymin,float ymax,
        float zmin,float zmax):
    xmin_(xmin),xmax_(xmax),ymin_(ymin),ymax_(ymax),zmin_(zmin),zmax_(zmax){
}
bool AABBBox::Intersect(Ray ray){
//    if(Inside(ray.origin)){
//        return true;
//    }
    float t;
    if(ray.direction.x !=0.f){
        //x方向求交点，交点是否在yz组成的四边形里
        if(ray.direction.x>0){
            t = (xmin_ - ray.origin.x)/ray.direction.x;
        }else{
            t = (xmax_ -ray.origin.x)/ray.direction.x;
        }
        if(t>0){
            Vector3 intersect_pt = ray.origin+t*ray.direction;
            if(ymin_<intersect_pt.y&&intersect_pt.y<ymax_&&
                zmin_<intersect_pt.z&&intersect_pt.z<zmax_){
                return true;
            }
        }
    }
    if(ray.direction.y !=0.f){
        if(ray.direction.y>0){
            t = (ymin_ - ray.origin.y)/ray.direction.y;
        }else{
            t = (ymax_ -ray.origin.y)/ray.direction.y;
        }
        if(t>0){
            Vector3 intersect_pt = ray.origin+t*ray.direction;
            if(xmin_<intersect_pt.x&&intersect_pt.x<xmax_&&
                zmin_<intersect_pt.z&&intersect_pt.z<zmax_){
                return true;
            }
        }
    }
    if(ray.direction.z !=0.f){
        if(ray.direction.z>0){
            t = (zmin_ - ray.origin.z)/ray.direction.z;
        }else{
            t = (zmax_ -ray.origin.z)/ray.direction.z;
        }
        if(t>0){
            Vector3 intersect_pt = ray.origin+t*ray.direction;
            if(ymin_<intersect_pt.y&&intersect_pt.y<ymax_&&
                xmin_<intersect_pt.x&&intersect_pt.x<xmax_){
                return true;
            }
        }
    }
    return false;
}
bool AABBBox::Intersect(std::shared_ptr<AABBBox> box){
    return (xmin_<=box->Xmax()&&xmax_>=box->Xmin())&&
           (ymin_<=box->Ymax()&&ymax_>=box->Ymin())&&
           (zmin_<=box->Zmax()&&zmax_>=box->Zmin());
}
std::shared_ptr<AABBBox> AABBBox::Combine(std::shared_ptr<AABBBox> box){
    float xmin = std::min( box->Xmin(),xmin_);
    float xmax = std::max(box->Xmax(),xmax_);

    float ymin = std::min( box->Ymin(),ymin_);
    float ymax = std::max(box->Ymax(),ymax_);

    float zmin = std::min( box->Zmin(),zmin_);
    float zmax = std::max(box->Zmax(),zmax_);

    //assert(0&&"not implement");
    return std::make_shared<AABBBox>(xmin,xmax,ymin,ymax,zmin,zmax);
}
std::shared_ptr<AABBBox> AABBBox::Copy(){
    return std::make_shared<AABBBox>(xmin_,xmax_,ymin_,ymax_,zmin_,zmax_);
}
bool AABBBox::Inside(Vector3 point){
    return (xmin_<= point.x&& point.x<=xmax_)&&
           (ymin_<= point.y&& point.y<=ymax_)&&
           (zmin_<= point.z&& point.z<=zmax_);
}
//void SubMesh::SetDrawMode(MeshDrawMode mode) {
//    draw_mode_ = mode;
//}
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
void SubMesh::AddData(MeshDataType type,unsigned char* data,size_t length)
{
    std::vector<unsigned char> vec_array(data,data+length);
    AddData(type,vec_array);
}
void SubMesh::AddData(MeshDataType type,VertexDataInfo& data)
{
    switch (type) {
        case MeshDataType::Mesh_Position:
            position_buffer_ = std::move(data);
            GenerateBoundingBox();
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
void SubMesh::GenerateBoundingBox(){
    if(position_buffer_.type == BufferType::TYPE_FLOAT){
        std::vector<Vector3> vertexs;
        vertexs.resize(position_buffer_.nums);
        assert(sizeof(Vector3)*position_buffer_.nums == position_buffer_.buffer.size());
        memcpy(vertexs.data(),position_buffer_.buffer.data(),
               sizeof(Vector3)*position_buffer_.nums);

        float xmin = std::numeric_limits<float>::max();
        float ymin = xmin,zmin = xmin;
        float xmax = std::numeric_limits<float>::min();
        float ymax = xmax,zmax=xmax;
        for(auto& vert:vertexs){
            xmin = std::min(xmin,vert.x);
            xmax = std::max(xmax,vert.x);

            ymin = std::min(ymin,vert.y);
            ymax = std::max(ymax,vert.y);

            zmin = std::min(zmin,vert.z);
            zmax = std::max(zmax,vert.z);
        }
        bounding_box_= std::make_shared<AABBBox>(xmin,xmax,ymin,ymax,zmin,zmax);
    }else{
        LogError("Other type not implement");
    }
}
void SubMesh::SetLocalMatrix(Matrix4 mat) {
    local_mat_ =mat;
}
ComponentType MeshComponent::GetType() { return Component_Mesh; }
void MeshComponent::AddSubMesh(std::shared_ptr<SubMesh> mesh){
    if(meshes_.empty()){
        bounding_box_ = mesh->BoundingBox();
    }else{
        bounding_box_ = mesh->BoundingBox()->Combine(bounding_box_);
    }
    meshes_.push_back(mesh);
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