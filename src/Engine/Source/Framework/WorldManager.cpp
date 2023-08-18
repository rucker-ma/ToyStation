//
// Created by ma on 2023/3/19.
//
#include "WorldManager.h"
#include "File/FileUtil.h"

#include <iostream>
namespace toystation{

void WorldManager::Initialize() {
    LoadLevel();
}
void WorldManager::PostInitialize(){
    current_level_->CheckLoadResult();
}
void WorldManager::Tick(){
    current_level_->Tick();
}
std::shared_ptr<Level> WorldManager::ActiveLevel(){
    return current_level_;
}
void WorldManager::InputTest(int num,double db,std::string str){
    std::cout<< num<<db<<str<<std::endl;
}
void WorldManager::LoadLevel() {
    current_level_ = std::make_shared<Level>();
    current_level_->Load(FileUtil::Combine("resource/level.json"));
}
}