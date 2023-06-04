//
// Created by ma on 2023/3/18.
//
#include "Level.h"

#include "Base/Global.h"
#include "Base/Logger.h"
#include "File/glTFLoader.h"
#include "Render/RenderSystem.h"

namespace toystation {

struct TObjectInfoReader {
    std::string name;
    std::string asset_path;
    TObjectInfoReader(Json::Value value) {
        name = value["name"].asString();
        asset_path = value["asset"].asString();
    }
};
BVHNode::BVHNode(std::shared_ptr<AABBBox> box){
    boundingbox = box->Copy();
    isleaf = false;
}
BVHLeafNode::BVHLeafNode(std::shared_ptr<TObject> object)
: BVHNode(object->GetComponent<MeshComponent>()->BoundingBox()){
    obj =object;
    isleaf = true;
}

BVHTree::BVHTree(){
}
void BVHTree::AddObject( std::shared_ptr<TObject> object){
    auto mesh = object->GetComponent<MeshComponent>();
    if(!mesh){
        return;
    }
    if(!root){
        root = std::make_shared<BVHNode>(mesh->BoundingBox());
        root->left = std::make_shared<BVHLeafNode>(object);
        return;
    }
    //添加节点与当前节点相交
    if(root->boundingbox->Intersect(mesh->BoundingBox())){
        if(root->isleaf){
            AddToTree(object,root);
        }else{
            bool join_left = (root->left!= nullptr)&&(root->left->boundingbox->Intersect(mesh->BoundingBox()));
            bool join_right = (root->right != nullptr)&&(root->right->boundingbox->Intersect(mesh->BoundingBox()));
            //既与左相交又与右相交
            if((join_left && join_right) ||
                ((!join_left)&&(!join_right))){
                //依次选择左右
                if(choose_left){
                    AddToTree(object,root->left);
                }else{
                    AddToTree(object,root->right);
                }
                choose_left = !choose_left;
            }else if(join_left){
                AddToTree(object,root->left);
            }else{
                AddToTree(object,root->right);
            }
        }
    }else{
        AddToTree(object,root);
    }
}
void BVHTree::AddToTree(std::shared_ptr<TObject> object,std::shared_ptr<BVHNode>root){
    auto mesh = object->GetComponent<MeshComponent>();
    std::shared_ptr<BVHNode> leafnode = std::make_shared<BVHLeafNode>(object);
    auto node = std::make_shared<BVHNode>(mesh->BoundingBox()->Combine(root->boundingbox));
    node->parent = root->parent;
    node->left = root;
    node->right = leafnode;
    leafnode->parent = node;
    root->parent = node;
}
std::shared_ptr<TObject> BVHTree::HitObject(Ray ray){
    return HitNode(root,ray);
}
std::shared_ptr<TObject> BVHTree::HitNode(std::shared_ptr<BVHNode> node, Ray ray){
    if(!node){
        return nullptr;
    }
    if(node->boundingbox->Intersect(ray)){
        if(node->isleaf){
            return std::dynamic_pointer_cast<BVHLeafNode>(node)->obj;
        }
        auto left_hit_result = HitNode(node->left,ray);
        if(left_hit_result){
            return left_hit_result;
        }
        auto right_hit_result = HitNode(node->right,ray);
        if(right_hit_result){
            return right_hit_result;
        }
    }
    return nullptr;
}
void Level::Load(std::string path) {
    JsonParseHelper parser;
    std::vector<char> data;
    FileUtil::ReadBinary(path, data);
    Json::Value value;
    if (!parser.parse(data.data(), data.size(), value)) {
        LogFatal("read level error");
    }
    assert(value["objects"].isArray());
    Json::Value ojects_ref = value["objects"];
    bvh_root_ = std::make_shared<BVHTree>();
    //std::vector<std::shared_ptr<TaskFuture>> results;
    for (int i = 0; i < ojects_ref.size(); ++i) {
        TObjectInfoReader reader(ojects_ref[i]);
        std::shared_ptr<TaskFuture> task_future = LoadObjectWithTask(reader);
        if(task_future) {
            task_results_.push_back(task_future);
        }
    }
    Json::Value environment = value["environment"];
    if(!environment.empty())
    {
        LoadMap(MapType::Map_Environment,environment["skybox"],task_results_);
        LoadMap(MapType::Map_Irradiance,environment["irradiance"],task_results_);
        LoadMap(MapType::Map_Radiance,environment["radiance"],task_results_);
        LoadMap(MapType::Map_BrdfLut,environment["brdf_lut"],task_results_);
    }
    controller_ =std::make_shared<EditorController>();
    controller_->Init();
}
void Level::CheckLoadResult(){
    for(auto& future:task_results_){
        future->Wait();
    }
    task_results_.clear();
}
void Level::LoadMap(MapType type,Json::Value value, std::vector<std::shared_ptr<TaskFuture>>& results){
    std::string path = value.asString();
    if(path.empty()){
        return;
    }
    std::string full_path =FileUtil::Combine(path);

    auto task = std::make_shared<TaskMessage<std::function<void()>>>(
        kRenderTaskID,[full_path,type]{
            //此处的执行会在渲染线程中执行
            RenderSystem::kRenderGlobalData.LoadMapAsTexture(type,full_path);
        }
    );
    kMesssageQueue.Post(kRendThread.get_id(),task);
    results.push_back(task->GetFuture());
}
std::shared_ptr<TaskFuture> Level::LoadObjectWithTask(Level::TObjectInfoReader& object_info) {
    if (!object_info.asset_path.empty()) {
        std::shared_ptr<TObject> obj = std::make_shared<TObject>(object_info.name);
        GltfModelLoader loader;
        loader.Load(object_info.asset_path, obj);
        objects_.push_back(obj);
        bvh_root_->AddObject(obj);
        // dispatch to rhi,create render handle
        auto task = std::make_shared<TaskMessage<std::function<void()>>>(
            kRenderTaskID,[obj]{
                //此处的执行会在渲染线程中执行
                RenderSystem::kRenderGlobalData.AddRenderObject(obj);
            }
            );
        kMesssageQueue.Post(kRendThread.get_id(),task);
        return task->GetFuture();
    }
    return nullptr;
}
void Level::Tick() {
    controller_->Tick();
}
void Level::BoundingBoxHit(Ray ray){
    std::shared_ptr<TObject> hited_obj = bvh_root_->HitObject(ray);
    if(hited_obj){
        MarkSelected(hited_obj);
    }
}
void Level::MarkSelected(std::shared_ptr<TObject> obj){
    if(selected_objects_.find(obj)==selected_objects_.end()){
        selected_objects_.insert(obj);
    }
}
std::shared_ptr<EditorController> Level::GetController(){
    assert(controller_);
    return controller_;
}
Level::TObjectInfoReader::TObjectInfoReader(Json::Value value) {
    name = value["name"].asString();
    asset_path = value["asset"].asString();
}
}  // namespace toystation