//
// Created by ma on 2023/8/2.
//

#include "Foo.h"
#include <iostream>

namespace toystation{

void OtherObj::GetObj(){
    std::cout<<"Call Get Obj"<<std::endl;
}
Foo::Foo(){
    std::cout<<"Foo 1"<<std::endl;
}
Foo::Foo(int a,float b){
    var_a_ = a;
    std::cout<<"Foo 2"<<std::endl;
}
void Foo::Print(){
    std::cout<<"Print"<<std::endl;
}
void Foo::InputSharedPtr(std::shared_ptr<OtherObj> obj){
    obj->GetObj();
}
void Foo::InputVectorValue(std::vector<int> vec){
    std::cout<<"Vector Value call: "<<std::endl;
    for(auto& v:vec){
        std::cout<< v<<std::endl;
    }
}
void Foo::InputVectorRef(std::vector<int>& vec){
    std::cout<<"Vector Ref call: "<<std::endl;
    for(auto& v:vec){
        std::cout<< v<<std::endl;
    }
}
}