//
// Created by ma on 2023/7/13.
//

#pragma once
#include <node/node.h>

namespace toystation{
class ObjectWrapper{
public:
    static void WrapV8Global(v8::Isolate* isolate,v8::Local<v8::Context> context);
private:
    static void WrapConsole(v8::Isolate* isolate,v8::Local<v8::Context> context);
};

}
