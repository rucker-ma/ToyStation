//
// Created by ma on 2023/7/13.
//
#include "ObjectWrapper.h"
#include "Base/Logger.h"
#include "ScriptUtils.h"
#include "Reflection/Meta.h"
#include "ToyEngine.h"

namespace toystation{

namespace script {
void ConsoleLogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() > 0) {
        v8::Isolate* isolate = args.GetIsolate();
        v8::String::Utf8Value message(isolate, args[0]);
        std::string log_message(*message);
        ScriptInfo(log_message);
    }
}

void ObjectGet(v8::Local<v8::Name> name,const v8::PropertyCallbackInfo<v8::Value>& info){
    v8::Isolate* isolate = info.GetIsolate();
    v8::String::Utf8Value require_name(isolate,name);
    v8::Local<v8::External> meta_wrap = v8::Local<v8::External>::Cast(info.Holder()->GetInternalField(0));
    auto* meta = static_cast<MetaClass*>( meta_wrap->Value());
    auto func = meta->GetFunction(*require_name);

    v8::Local<v8::Value> meta_func = v8::External::New(isolate,&func);
    v8::Local<v8::Function> func_wrap = v8::Function::New(info.GetIsolate()->GetCurrentContext(),[](const v8::FunctionCallbackInfo<v8::Value>& info){
    auto meta_func = static_cast<MetaFunction*>( info.Data().As<v8::External>()->Value());
     meta_func->Invoke(kEngine);
    },meta_func).ToLocalChecked();
    info.GetReturnValue().Set(func_wrap);

}

void ObjectSet(v8::Local<v8::Name> name,v8::Local<v8::Value> value,const v8::PropertyCallbackInfo<v8::Value>& info){

}

}

void ObjectWrapper::WrapV8Global(v8::Isolate* isolate,v8::Local<v8::Context> context){
    WrapConsole(isolate,context);
    WrapEngine(isolate,context);
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
void ObjectWrapper::WrapEngine(v8::Isolate* isolate,v8::Local<v8::Context> context){
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> global = context->Global();

    v8::Local<v8::ObjectTemplate> result_templ = v8::ObjectTemplate::New(isolate);
    result_templ->SetInternalFieldCount(1);
    result_templ->SetHandler(v8::NamedPropertyHandlerConfiguration(script::ObjectGet,script::ObjectSet));
    v8::Local<v8::Object>result = result_templ->NewInstance(context).ToLocalChecked();
    result->SetInternalField(0,v8::External::New(isolate,&ToyEngine::Class()));
    global->Set(context, ScriptUtils::ToV8String(isolate, "toy"), result);
}
void ObjectWrapper::CreateTemplate(v8::Isolate* isolate,v8::Local<v8::Context> context){
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> global = context->Global();
    metavar_template = v8::ObjectTemplate::New(isolate);
    metavar_template->SetInternalFieldCount(1);
    metafunc_template = v8::FunctionTemplate::New(isolate);
}
}