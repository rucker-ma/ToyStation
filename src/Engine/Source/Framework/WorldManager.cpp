//
// Created by ma on 2023/3/19.
//
#include "WorldManager.h"
#include "File/FileUtil.h"


namespace toystation{

void WorldManager::Initialize() {
    LoadLevel();
}
void WorldManager::Tick(){

}

void WorldManager::LoadLevel() {
    current_level_ = std::make_shared<Level>();
    current_level_->Load(FileUtil::Combine("resource/level.json"));
}
}