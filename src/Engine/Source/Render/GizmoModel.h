//
// Created by ma on 2023/5/18.
//

#pragma once
#include <vector>
#include "Framework/Component/MeshComponent.h"
#include "Framework/TObject.h"
namespace toystation{
class GizmoModel:public TObject{
public:
    virtual ~GizmoModel(){}
    virtual void GenerateModel(){}
    virtual std::vector<std::shared_ptr<SubMesh>> GetMeshes() = 0;
};

class MoveGizmoModel:public GizmoModel{
public:
    MoveGizmoModel();
    virtual ~MoveGizmoModel();
    void GenerateModel()override;
    std::vector<std::shared_ptr<SubMesh>> GetMeshes()override;
private:
    void Generate(Matrix3 rotate);
    void AddOrigin();
private:
    float arrow_radius_;
    float arrow_length_;
    float axis_length_;
    float axis_radius_;
    int samples_;//一个圆周上采样多少个点,指定为偶数
};

class RotateGizmoModel:public GizmoModel{
public:
    RotateGizmoModel();
    virtual ~RotateGizmoModel();
    void GenerateModel()override;
    std::vector<std::shared_ptr<SubMesh>> GetMeshes()override;
private:
    void Generate(Matrix3 rotate);
private:
    float large_radius_;
    float small_radius_;
    int samples_;
    std::vector<std::shared_ptr<SubMesh>> meshes_;
};

//class ScaleGizmoModel:public GizmoModel{
//
//
//};
}