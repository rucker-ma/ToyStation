//
// Created by ma on 2023/6/6.
//
#include "BVHTree.h"

namespace toystation{

BVHNode::BVHNode(AABBBox box){
    boundingbox = box;
    isleaf = false;
    split_axis = AxisNone;
}
BVHLeafNode::BVHLeafNode(std::shared_ptr<SubMesh> object)
    : BVHNode(object->BoundingBox()){
    mesh =object;
    isleaf = true;
}

BVHTree::BVHTree(){
    std::random_device rd;
    gen_=  std::mt19937(rd());
    dist_ = std::uniform_real_distribution<double>(-1,1);
}
void BVHTree::BuildTree(
    std::vector<std::shared_ptr<TObject>> objects){
    std::vector<std::shared_ptr<SubMesh>> meshes;
    for(auto obj:objects){
        auto submeshes = obj->GetComponent<MeshComponent>()->GetSubMesh();
        meshes.insert(meshes.end(),submeshes.begin(),submeshes.end());
    }
    Build(meshes, nullptr,true);
}
void BVHTree::Build(std::vector<std::shared_ptr<SubMesh>> meshes,
                    std::shared_ptr<BVHNode> parent_node,bool left){
    assert(!meshes.empty());
    if(meshes.size() == 1){
        std::shared_ptr<BVHNode> leaf_node = std::make_shared<BVHLeafNode>(meshes[0]);
        leaf_node->parent = parent_node;
        if(left){
            parent_node->left = leaf_node;
        }else{
            parent_node->right = leaf_node;
        }
        auto vertex = meshes[0]->BoundingBox().OutlineVertex();
        bounding_vertex_.insert(bounding_vertex_.end(),vertex.begin(),vertex.end());
        return;
    }
    AABBBox box = meshes[0]->BoundingBox();
    for(auto& mesh:meshes){
        box = mesh->BoundingBox().Combine(box);
    }

    std::shared_ptr<BVHNode> node = std::make_shared<BVHNode>(box);
//    auto vertex = box.OutlineVertex();
//    bounding_vertex_.insert(bounding_vertex_.end(),vertex.begin(),vertex.end());

    Vector3 length = {box.Xmax()-box.Xmin(),box.Ymax()-box.Ymin(),box.Zmax()-box.Zmin()};


    if(parent_node == nullptr){
        root =  std::make_shared<BVHNode>(box);
        node = root;
        parent_node = root;
    }else {
        node->parent = parent_node;
        if (left) {
            parent_node->left = node;
        } else {
            parent_node->right = node;
        }
    }

    float maxlength = std::max({length.x,length.y,length.z});
    int compare_axis =2;
    Vector3 box_center = node->boundingbox.Center();
    if(maxlength == length.x){
            compare_axis = 0;
    }else if(maxlength == length.y){
            compare_axis = 1;
    }
    std::vector<std::shared_ptr<SubMesh>> left_objects;
    std::vector<std::shared_ptr<SubMesh>> right_objects;

    for(int i =0;i<3;i++){
        left_objects.clear();
        right_objects.clear();
        node->split_axis = static_cast<BVHNode::SplitAxis>(compare_axis);
        double random_num = dist_(gen_);
        // 添加一个随机扰动，避免死循环
        box_center[compare_axis] += random_num * length[compare_axis] / 100;
        for (auto& mesh : meshes) {
        auto mesh_center = mesh->BoundingBox().Center();
            if (mesh_center[compare_axis] <= box_center[compare_axis]) {
                left_objects.push_back(mesh);
            } else {
                right_objects.push_back(mesh);
            }
        }
        if(!(left_objects.empty()||right_objects.empty())){
            break;
        }
        //一次分割失败，切换分割轴
        compare_axis = (compare_axis+1)%3;
    }

    if(left_objects.empty()||right_objects.empty()){
        left_objects.clear();
        right_objects.clear();
        //三次切换轴分割失败，直接排序分割
        std::sort(meshes.begin(),meshes.end(),[compare_axis](std::shared_ptr<SubMesh> i,
                                                               std::shared_ptr<SubMesh> j){
            return (i->BoundingBox().Center()[compare_axis])<= (j->BoundingBox().Center()[compare_axis]);
        });
        int half = (meshes.size()+1)/2;
        left_objects.insert(left_objects.end(), meshes.begin(),meshes.begin()+half);
        right_objects.insert(right_objects.end(), meshes.begin()+half,meshes.end());
    }
    Build(left_objects,node, true);
    Build(right_objects,node, false);
}
HitResult BVHTree::HitObject(Ray ray){
    return HitNode(root,ray);
}
HitResult BVHTree::HitNode(std::shared_ptr<BVHNode> node, Ray ray){
    if(!node){
        return HitResult::HitNone();
    }
    IntersectRes intersect_res = node->boundingbox.Intersect(ray);

    if(intersect_res.intersect) {
        if(node->isleaf){
            auto mesh = std::dynamic_pointer_cast<BVHLeafNode>(node)->mesh;
            intersect_res = mesh->Intersect(ray);
            HitResult res{ mesh->Parent()->Parent(),mesh,intersect_res.intersect ,intersect_res.time,ray};
            return res;
        }
        auto left_hit_result = HitNode(node->left,ray);
        auto right_hit_result = HitNode(node->right,ray);
        if(left_hit_result.hit && right_hit_result.hit){
            return left_hit_result.time<right_hit_result.time?left_hit_result:right_hit_result;
        }
        if(left_hit_result.hit){
            return left_hit_result;
        }
        if(right_hit_result.hit){
            return right_hit_result;
        }
    }
    return HitResult::HitNone();
}
}