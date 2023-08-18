//
// Created by ma on 2023/8/2.
//

#include "Gen.h"

#include "Foo.h"

BEGIN_DEFINE(Foo)
    .AddConstruct<>()
    .AddConstruct<int,float>()
    .AddVariable("var_a_", &Foo::var_a_)
    .AddFunction("Print", &Foo::Print)
    .AddFunction("InputSharedPtr", &Foo::InputSharedPtr)
    .AddFunction("InputVectorValue", &Foo::InputVectorValue)
    .AddFunction("InputVectorRef", &Foo::InputVectorRef)
END_DEFINE()