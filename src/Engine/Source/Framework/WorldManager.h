//
// Created by ma on 2023/3/19.
//
#pragma once
#include "Level.h"

namespace toystation{

class WorldManager{
public:
    void Initialize();
    void Tick();
    std::shared_ptr<Level> ActiveLevel();
private:
    void LoadLevel();
private:
    std::shared_ptr<Level> current_level_;
};
}
