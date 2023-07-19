//
// Created by ma on 2023/3/19.
//
#pragma once
#include "Level.h"
#include "Reflection/Meta.h"

namespace toystation {

class WorldManager {
    GENERATE_BODY(WorldManager)
public:
    void Initialize();
    void PostInitialize();
    void Tick();
    std::shared_ptr<Level> ActiveLevel();

private:
    void LoadLevel();

private:
    std::shared_ptr<Level> current_level_;
};
}  // namespace toystation
