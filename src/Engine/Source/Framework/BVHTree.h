//
// Created by ma on 2023/6/6.
//
#include <random>
#include "Component/MeshComponent.h"
#include "TObject.h"

namespace toystation{

class BVHNode{
public:
    enum SplitAxis{
        AxisX,
        AxisY,
        AxisZ,
        AxisNone
    };
    BVHNode(AABBBox box);
    virtual ~BVHNode(){}
    std::shared_ptr<BVHNode> left;
    std::shared_ptr<BVHNode> right;
    std::shared_ptr<BVHNode> parent;
    AABBBox boundingbox;
    bool isleaf;
    SplitAxis split_axis;
};
class BVHLeafNode:public BVHNode{
public:
    virtual ~BVHLeafNode(){}
    BVHLeafNode(std::shared_ptr<SubMesh> object);
    std::shared_ptr<SubMesh> mesh;
};

class HitResult{
public:
    std::shared_ptr<TObject> hit_object;
    std::shared_ptr<SubMesh> hit_mesh;
    bool hit;
    float time;
    Ray ray;
    static HitResult HitNone(){
        HitResult res;
        res.hit =false;
        res.time =0;
        return res;
    }
};

//TODO:如何更好的构建树有待调整
class BVHTree{
public:
    BVHTree();
    HitResult HitObject(Ray ray);
    void BuildTree(std::vector<std::shared_ptr<TObject>> objects);
    const std::vector<Vector3>& BoundingVertex(){ return bounding_vertex_;}
private:
//    void Build(std::vector<std::shared_ptr<TObject>> objects, std::shared_ptr<BVHNode> parent_node,bool left);
    void Build(std::vector<std::shared_ptr<SubMesh>> meshes, std::shared_ptr<BVHNode> parent_node,bool left);
    HitResult HitNode(std::shared_ptr<BVHNode> node, Ray ray);
    std::vector<Vector3> bounding_vertex_;
    std::mt19937 gen_;
    std::uniform_real_distribution<double> dist_;
public:
    std::shared_ptr<BVHNode> root;
};

}
