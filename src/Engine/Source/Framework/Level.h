//
// Created by ma on 2023/3/18.
//

#pragma once
#include <string>
#include <vector>

#include "Base/MessageQueue.h"

#include "File/FileUtil.h"
#include "TObject.h"
#include "MeshComponent.h"
#include "EditorController.h"

namespace toystation{
enum class MapType{
    Map_Environment,
    Map_Radiance,
    Map_Irradiance,
    Map_BrdfLut
};

class BVHTree{
public:
    void AddObject( std::shared_ptr<TObject> object);
    std::shared_ptr<TObject> HitObject(Ray ray);
public:
    std::shared_ptr<BVHTree> left;
    std::shared_ptr<BVHTree> right;
    std::shared_ptr<AABBBox> box;
    std::shared_ptr<TObject> object;
};

class Level{
public:
    void Load(std::string path);
    void CheckLoadResult();
    void Tick();
    void BoundingBoxHit(Ray ray);
    std::shared_ptr<EditorController> GetController();
private:
    struct TObjectInfoReader{
        std::string name;
        std::string asset_path;
        TObjectInfoReader(Json::Value value);
    };
    void LoadMap(MapType type,Json::Value value, std::vector<std::shared_ptr<TaskFuture>>& results);
    std::shared_ptr<TaskFuture> LoadObjectWithTask(TObjectInfoReader& object_info);
    void MarkSelected(std::shared_ptr<TObject> obj);
private:
    std::vector<std::shared_ptr<TObject>> objects_;
    std::shared_ptr<EditorController> controller_;
    std::vector<std::shared_ptr<TaskFuture>> task_results_;
    std::shared_ptr<BVHTree> bvh_root_;
};
}