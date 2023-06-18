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
    bvh_root_->BuildTree(objects_);
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
    HitResult hited_res = bvh_root_->HitObject(ray);
    if(hited_res.hit){
        hit_result_ = hited_res;
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