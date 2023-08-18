//
// Created by ma on 2023/3/19.
//
#pragma once
#include "Level.h"
#include "Meta/Meta.h"

namespace toystation {

class WorldManager {
    GENERATE_BODY(WorldManager)
public:
    void Initialize();
    void PostInitialize();
    void Tick();
    std::shared_ptr<Level> ActiveLevel();
    void InputTest(int num,double db,std::string str);
private:
    void LoadLevel();

private:
    std::shared_ptr<Level> current_level_;
};
}  // namespace toystation
