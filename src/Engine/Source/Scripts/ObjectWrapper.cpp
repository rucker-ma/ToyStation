//
// Created by ma on 2023/7/13.
//
#include "ObjectWrapper.h"
#include "Base/Logger.h"
#include "Meta/Meta.h"
#include "ToyEngine.h"

namespace toystation{

namespace script {

reflect::MetaArg V8ValueToMeta(v8::Isolate* isolate,v8::Local<v8::Value> value){
    if(value->IsNumber()){
        auto number = value.As<v8::Number>();
        return reflect::MetaArg(number->Value());
    }
    if(value->IsString()){
        auto str = value.As<v8::String>();
        return reflect::MetaArg(ScriptUtils::ToStdString(isolate,str));
    }
    if(value->IsObject()){
        auto obj = value.As<v8::Object>();
    }
    return reflect::MetaArg();
}


//MetaArg封装类型的参数调用
static void MetaArgCallback(const v8::FunctionCallbackInfo<v8::Value>& info){
    int arg_size = info.Length();
    LogWarn("Exec func call ");
    v8::Isolate* isolate = info.GetIsolate();
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Context::Scope context_scope(context);

    v8::Local<v8::External> meta_wrap = info.This()->GetInternalField(0).As<v8::External>();
    auto* meta_arg = static_cast<reflect::MetaArg*>( meta_wrap->Value());

    auto* meta_func =static_cast<MetaFunction*>(info.Data().As<v8::External>()->Value());
    std::vector<reflect::MetaArg> inputs;
    inputs.push_back(*meta_arg);
    assert(meta_func->ArgNum() == arg_size);
    for (int i = 0; i < arg_size; ++i) {
        if(info[i]->IsNullOrUndefined()){
            LogError("Script input arg is null or undefined");
            return;
        }
        inputs.push_back(V8ValueToMeta(isolate, info[i]));
    }

    reflect::MetaArg ret = meta_func->Invoke(inputs);
    if(ret.Meta() == nullptr){
        return;
    }
    v8::Local<v8::Object> obj = ObjectWrapper::WrapMetaArg(isolate,ret);
    info.GetReturnValue().Set(obj);
}

static void CleanMetaArg(const v8::WeakCallbackInfo<reflect::MetaArg>& data){
    //should clean data
    //data.GetInternalField(0);
    LogWarn("Free MetaArg Object");
    auto meta_arg = data.GetParameter();
    delete meta_arg;
    meta_arg = nullptr;
}

void ConsoleLogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() > 0) {
        v8::Isolate* isolate = args.GetIsolate();
        v8::String::Utf8Value message(isolate, args[0]);
        std::string log_message(*message);
        ScriptInfo(log_message);
    }
}
}
std::map<std::string,v8::UniquePersistent<v8::FunctionTemplate>> ObjectWrapper::type_templates = {};
std::map<reflect::MetaArg*,v8::UniquePersistent<v8::Value>> ObjectWrapper::persistent_obj_cache_ = {};
void ObjectWrapper::WrapV8Global(v8::Isolate* isolate,v8::Local<v8::Context> context){
    WrapConsole(isolate,context);
    //WrapEngine(isolate,context);
}
v8::Local<v8::FunctionTemplate> ObjectWrapper::MakeMetaTemplate(v8::Isolate* isolate,MetaClass* meta){
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> result = v8::FunctionTemplate::New(isolate);

    //提取MetaClass信息，添加到template中
    for(auto& func: meta->GetFunctions()){
        auto func_wrap = v8::External::New(isolate,const_cast<MetaFunction*>(&func));
        auto meta_func_call = v8::FunctionTemplate::New(isolate,script::MetaArgCallback,func_wrap);
        result->PrototypeTemplate()->Set(isolate,func.Name().c_str(),meta_func_call);
    }
    result->SetClassName(ScriptUtils::ToV8String(isolate,meta->Name().c_str()));
    result->InstanceTemplate()->SetInternalFieldCount(1);

    v8::UniquePersistent<v8::FunctionTemplate> func_template = v8::UniquePersistent<v8::FunctionTemplate>(isolate,result);
    type_templates[meta->Name()] = std::move(func_template);

    return handle_scope.Escape(result);
}
v8::Local<v8::Object> ObjectWrapper::WrapMetaArg(v8::Isolate* isolate, reflect::MetaArg& arg){
    v8::Isolate::Scope isolate_scope(isolate);
    v8::EscapableHandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Context::Scope context_scope(context);
    if(type_templates.find(arg.Meta()->Name())== type_templates.end()){
        MakeMetaTemplate(isolate,arg.Meta());
    }
    auto func_template = type_templates[arg.Meta()->Name()].Get(isolate);

    v8::Local<v8::Object> obj = func_template->InstanceTemplate()->NewInstance(
                               isolate->GetCurrentContext()).ToLocalChecked();
    //实例化设置对象
    //TODO: 这个包裹了实际对象的对象怎么保证生命周期？
    //v8::Local<v8::ObjectTemplate> wrap_ret_template =  v8::ObjectTemplate::New(isolate);
    //wrap_ret_template->SetInternalFieldCount(1);
    //v8::Local<v8::Object> wrap_ret = meta_arg_template->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
    reflect::MetaArg* ret_ptr = new reflect::MetaArg(arg);
    v8::Local<v8::Value> external_wrap = v8::External::New(isolate,ret_ptr);
    obj->SetInternalField(0,external_wrap);

    //TODO:add to global cache
    v8::UniquePersistent<v8::Value> value;
    value.Reset(isolate,obj);
    value.SetWeak<reflect::MetaArg>(ret_ptr,script::CleanMetaArg,v8::WeakCallbackType::kParameter);
    persistent_obj_cache_[ret_ptr] = std::move(value);
    return handle_scope.Escape(obj);
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