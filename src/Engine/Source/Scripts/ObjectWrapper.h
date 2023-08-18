//
// Created by ma on 2023/7/13.
//

#pragma once
#include <node/node.h>
#include <Meta/Meta.h>
#include <map>
#include "ScriptUtils.h"
namespace toystation{
class ObjectWrapper{
public:
    static void WrapV8Global(v8::Isolate* isolate,v8::Local<v8::Context> context);
    static v8::Local<v8::FunctionTemplate> MakeMetaTemplate(v8::Isolate* isolate,
                                                            MetaClass* meta);
    static v8::Local<v8::Object> WrapMetaArg(v8::Isolate* isolate,reflect::MetaArg& arg);

private:
    static void WrapConsole(v8::Isolate* isolate,v8::Local<v8::Context> context);
    static void WrapEngine(v8::Isolate* isolate,v8::Local<v8::Context> context);
    static std::map<std::string,v8::UniquePersistent<v8::FunctionTemplate>> type_templates;
    static std::map<reflect::MetaArg*,v8::UniquePersistent<v8::Value>> persistent_obj_cache_;
};

}
