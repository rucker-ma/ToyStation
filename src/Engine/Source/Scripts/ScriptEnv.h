//
// Created by ma on 2023/8/4.
//

#pragma once
#include <node/node.h>
#include <node/v8.h>
#include "ScriptUtils.h"
#include "ObjectWrapper.h"
namespace toystation{

class ScriptEnv{
public:
    ScriptEnv(std::unique_ptr<node::CommonEnvironmentSetup> setup);
    void Run();
    void ExecuteScript(std::string path);
    void Stop();

private:
    void InitGlobal();
    void LoadModule(const v8::FunctionCallbackInfo<v8::Value>& info);

private:
    std::unique_ptr<node::CommonEnvironmentSetup> setup_;
    using V8FunctionCallback = void(ScriptEnv::*)(const v8::FunctionCallbackInfo<v8::Value>& info);

    template <V8FunctionCallback callbck>
    static void V8Bind(v8::Isolate* isolate,v8::Local<v8::Context> context,
                       v8::Local<v8::Object> obj,const char* name,v8::Local<v8::External> self){
        auto func_template = v8::FunctionTemplate::New(isolate,
                                 [](const v8::FunctionCallbackInfo<v8::Value>& info){
                                     auto env = static_cast<ScriptEnv*>(v8::Local<v8::External>::Cast(info.Data())->Value());
                                     (env->*callbck)(info);
                                 },
                                 self);
        obj->Set(context,ScriptUtils::ToV8String(isolate,name),func_template->GetFunction(context).ToLocalChecked()).Check();
    }
};

}