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
    std::vector<std::shared_ptr<TaskFuture>> results;
    for (int i = 0; i < ojects_ref.size(); ++i) {
        TObjectInfoReader reader(ojects_ref[i]);
        std::shared_ptr<TaskFuture> task_future = LoadObjectWithTask(reader);
        if(task_future) {
            results.push_back(task_future);
        }
    }
    for(auto& future:results){
        future->Wait();
    }
    camera_ =std::make_shared<Camera>();
}
std::shared_ptr<TaskFuture> Level::LoadObjectWithTask(Level::TObjectInfoReader& object_info) {
    std::shared_ptr<TObject> obj = std::make_shared<TObject>(object_info.name);
    if (!object_info.asset_path.empty()) {
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
    camera_->Tick();
}
std::shared_ptr<Camera> Level::GetCamera(){
    assert(camera_);
    return camera_;
}
Level::TObjectInfoReader::TObjectInfoReader(Json::Value value) {
    name = value["name"].asString();
    asset_path = value["asset"].asString();
}
}  // namespace toystation