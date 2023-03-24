//
// Created by ma on 2023/3/23.
//
#pragma once
#include <memory>

namespace toystation{

class RenderMesh{

};

class RenderMaterial{

};

class RenderObject
{
public:
    RenderObject(int id);
    int GetID()const{return id_;}

private:
    int id_;
    std::shared_ptr<RenderMesh> mesh_;
    std::shared_ptr<RenderMaterial> material_;
};
}