//
// Created by ma on 2023/7/13.
//
#include "ScriptUtils.h"

namespace toystation{

v8::Local<v8::String> ScriptUtils::ToV8String(v8::Isolate* isolate,const char* string){
    return v8::String::NewFromUtf8(isolate, string, v8::NewStringType::kNormal).ToLocalChecked();
}
std::string ScriptUtils::ToStdString(v8::Isolate* isolate,v8::Local<v8::String> string){
    v8::String::Utf8Value utf8value(isolate, string);
    return std::string(*utf8value);
}
}