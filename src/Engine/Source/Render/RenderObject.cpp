//
// Created by ma on 2023/3/23.
//
#include "RenderObject.h"
#include <cassert>

namespace toystation{

RenderObject::RenderObject(int id){
    assert(id>0);
    id_ =id;
}

}