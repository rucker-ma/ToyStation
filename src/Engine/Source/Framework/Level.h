//
// Created by ma on 2023/3/18.
//

#pragma once
#include <string>
#include <vector>
#include "Base/MessageQueue.h"
#include "File/FileUtil.h"
#include "TObject.h"
#include "Camera.h"


namespace toystation{

class Level{
public:
    void Load(std::string path);
    void Tick();
    std::shared_ptr<Camera> GetCamera();
private:
    struct TObjectInfoReader{
        std::string name;
        std::string asset_path;
        TObjectInfoReader(Json::Value value);
    };

    std::shared_ptr<TaskFuture> LoadObjectWithTask(TObjectInfoReader& object_info);
private:
    std::vector<std::shared_ptr<TObject>> objects_;
    std::shared_ptr<Camera> camera_;
};
}