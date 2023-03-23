//
// Created by ma on 2023/3/17.
//
#include "TObject.h"
namespace toystation{
std::atomic<int> TObject::kIDAllocator(0);

TObject::TObject(std::string name):name_(name){
}
}