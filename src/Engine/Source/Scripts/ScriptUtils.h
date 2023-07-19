//
// Created by ma on 2023/7/13.
//

#pragma once
#include <node/v8.h>

namespace toystation{

class ScriptUtils{
public:
    static v8::Local<v8::String> ToV8String(v8::Isolate* isolate,const char* string);
    static std::string ToStdString(v8::Isolate* isolate,v8::Local<v8::String> string);
    static std::string ToStdString(v8::Isolate* isolate,v8::Local<v8::Value> value);
};
}
