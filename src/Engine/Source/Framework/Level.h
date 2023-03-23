//
// Created by ma on 2023/3/18.
//

#pragma once
#include <string>
#include <vector>
#include "File/FileUtil.h"
#include "TObject.h"


namespace toystation{

class Level{
public:
    void Load(std::string path);
    void Tick();
private:
    struct TObjectInfoReader{
        std::string name;
        std::string asset_path;
        TObjectInfoReader(Json::Value value);
    };

    void LoadObject(TObjectInfoReader& object_info);
private:
    std::vector<std::shared_ptr<TObject>> objects_;
};
}