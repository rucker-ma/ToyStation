//
// Created by ma on 2023/8/2.
//

#pragma once
#include "Meta.h"
#include <mutex>

namespace toystation {


class OtherObj{
public:
    void GetObj();
};

class Foo {
    GENERATE_BODY(Foo)
public:
    Foo();
    Foo(int a,float b);
    void Print();
    void InputSharedPtr(std::shared_ptr<OtherObj> obj);
    void InputVectorValue(std::vector<int> vec);
    void InputVectorRef(std::vector<int>& vec);
private:
    int var_a_;
    std::mutex mtx;
};
}
 // TOYSTATION_FOO_H
