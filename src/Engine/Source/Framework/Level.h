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
#include "MeshComponent.h"
#include "EditorController.h"

namespace toystation{
enum class MapType{
    Map_Environment,
    Map_Radiance,
    Map_Irradiance,
    Map_BrdfLut
};

class BVHNode{
public:
    BVHNode(std::shared_ptr<AABBBox> box);
    virtual ~BVHNode(){}
    std::shared_ptr<BVHNode> left;
    std::shared_ptr<BVHNode> right;
    std::shared_ptr<BVHNode> parent;
    std::shared_ptr<AABBBox> boundingbox;
    bool isleaf;
};
class BVHLeafNode:public BVHNode{
public:
    virtual ~BVHLeafNode(){}
    BVHLeafNode(std::shared_ptr<TObject> object);
    std::shared_ptr<TObject> obj;
};
//TODO:如何更好的构建树有待调整
class BVHTree{
public:
    BVHTree();
    void AddObject( std::shared_ptr<TObject> object);
    std::shared_ptr<TObject> HitObject(Ray ray);
private:
    std::shared_ptr<TObject> HitNode(std::shared_ptr<BVHNode> node, Ray ray);
private:
    void AddToTree(std::shared_ptr<TObject> object,std::shared_ptr<BVHNode>root);
public:
    //std::shared_ptr<BVHTree> left;
    //std::shared_ptr<BVHTree> right;
    std::shared_ptr<BVHNode> root;
//    std::shared_ptr<AABBBox> box;
   // std::shared_ptr<TObject> object;
    bool choose_left;
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
    std::set<std::shared_ptr<TObject>> selected_objects_;//存储选中的对象
};
}