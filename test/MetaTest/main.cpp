//
// Created by ma on 2023/8/2.
//
#include <gtest/gtest.h>
#include "Foo.h"

using namespace toystation;
//基本类型
TEST(Meta,basic_func){
    auto meta_foo = reflect::GetByName("Foo");
    auto func =  meta_foo.GetFunction("Print");
    Foo foo;
    func.Invoke({&foo});
}
TEST(Meta,func_input_shared_ptr){
    auto meta_foo = reflect::GetByName("Foo");
    auto func =  meta_foo.GetFunction("InputSharedPtr");
    Foo foo;
    func.Invoke({&foo,std::make_shared<OtherObj>()});
}
TEST(Meta,func_input_vector_value){
    auto meta_foo = reflect::GetByName("Foo");
    auto func =  meta_foo.GetFunction("InputVectorValue");
    Foo foo;
    std::vector<int> res{3,2,1};
    func.Invoke({&foo,res});
}
TEST(Meta,func_input_vector_ref){
    auto meta_foo = reflect::GetByName("Foo");
    auto func =  meta_foo.GetFunction("InputVectorRef");
    Foo foo;
    std::vector<int> res{1,2,3};
    func.Invoke({&foo,res});
}
//基本变量
TEST(Meta,basic_var){
    auto meta_foo = reflect::GetByName("Foo");
    auto var =  meta_foo.GetVariable("var_a_");
    Foo foo;

    var.SetValue(foo,3);
    int val = var.GetValue<int>(foo);
    ASSERT_EQ(3,val);
}
TEST(Meta,basic_var1){
    auto meta_foo = reflect::GetByName("Foo");
    auto var =  meta_foo.GetVariable("var_a_");
    Foo foo;

    var.SetValue(&foo,3);
    auto ret = var.GetValue(&foo);
    ASSERT_EQ(3,ret.Cast<int>());
}

TEST(Meta,Construct){
    auto meta_foo = reflect::GetByName("Foo");

    auto foo1 = meta_foo.GetConstruct(0)({});
    auto foo2 = meta_foo.GetConstruct(2)({1,3.5f});
    auto raw_foo1 = foo1.Cast<Foo*>();
    auto raw_foo2 = foo2.Cast<Foo*>();
}
int main(int argc,char** argv){
    testing::InitGoogleTest();
    RUN_ALL_TESTS();
    return 0;
}