//
// Created by ma on 2023/7/13.
//
#include "ObjectWrapper.h"
#include "Base/Logger.h"
#include "ScriptUtils.h"


namespace toystation{

namespace script {
void ConsoleLogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() > 0) {
        v8::Isolate* isolate = args.GetIsolate();
        v8::String::Utf8Value message(isolate, args[0]);
        std::string logMessage(*message);
        ScriptInfo(logMessage);
    }
}
}

void ObjectWrapper::WrapV8Global(v8::Isolate* isolate,v8::Local<v8::Context> context){
    WrapConsole(isolate,context);
}

void ObjectWrapper::WrapConsole(v8::Isolate* isolate,v8::Local<v8::Context> context){
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> global = context->Global();
    v8::Local<v8::Value> console = global->Get(context,ScriptUtils::ToV8String(isolate,"console")).ToLocalChecked();
    if(console->IsObject()) {
        //替换掉console.log函数
        console.As<v8::Object>()->Set(context,ScriptUtils::ToV8String(isolate,"log"),
        v8::FunctionTemplate::New(isolate,script::ConsoleLogCallback)->GetFunction(context).ToLocalChecked());
    }
}
}