//
// Created by ma on 2023/3/18.
//

#pragma once
#include <string>
#include <vector>
#include <set>
#include "Base/MessageQueue.h"

#include "File/FileUtil.h"
#include "TObject.h"
#include "Component/MeshComponent.h"
#include "EditorController.h"
#include "BVHTree.h"

namespace toystation{
enum class MapType{
    Map_Environment,
    Map_Radiance,
    Map_Irradiance,
    Map_BrdfLut
};



class Level{
public:
    void Load(std::string path);
    void CheckLoadResult();
    void Tick();
    void BoundingBoxHit(Ray ray);
    std::shared_ptr<EditorController> GetController();
    std::shared_ptr<BVHTree> BoundingTree(){return bvh_root_;}
    HitResult GetHitResult(){ return hit_result_;}

private:
    struct TObjectInfoReader{
        std::string name;
        std::string asset_path;
        TObjectInfoReader(Json::Value value);
    };
    void LoadMap(MapType type,Json::Value value, std::vector<std::shared_ptr<TaskFuture>>& results);
    std::shared_ptr<TaskFuture> LoadObjectWithTask(TObjectInfoReader& object_info);
//    void MarkSelected(std::shared_ptr<TObject> obj);
private:
    std::vector<std::shared_ptr<TObject>> objects_;
    std::shared_ptr<EditorController> controller_;
    std::vector<std::shared_ptr<TaskFuture>> task_results_;
    std::shared_ptr<BVHTree> bvh_root_;
    HitResult hit_result_;
};
}