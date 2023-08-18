//
// Created by ma on 2023/8/4.
//

#include "ScriptEnv.h"
#include "Base/Logger.h"
#include "File/FileUtil.h"
#include "Meta/Meta.h"
#include "ScriptUtils.h"
#include "ObjectWrapper.h"
#include "ToyEngine.h"

namespace toystation{



ScriptEnv::ScriptEnv(std::unique_ptr<node::CommonEnvironmentSetup> setup)
    :setup_(std::move(setup)){
}
void ScriptEnv::Run(){

    v8::Isolate* isolate = setup_->isolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(setup_->context());
    isolate->SetMicrotasksPolicy(v8::MicrotasksPolicy::kAuto);

    std::string init_script = R"(const publicRequire = require('module').createRequire(process.cwd()+'/dist/');
	globalThis.require = publicRequire;
	globalThis.exports = {};)";
    v8::MaybeLocal<v8::Value> load_ret = node::LoadEnvironment(setup_->env(), init_script.c_str());
    if(load_ret.IsEmpty()){
        LogError("Load node environment error");
    }
    //初始化js环境global
    InitGlobal();
    //运行入口脚本
    ExecuteScript("out/dist/entry.js");
    node::SpinEventLoop(setup_->env()).FromMaybe(1);
}
void ScriptEnv::ExecuteScript(std::string path){

    std::string file_path = FileUtil::Combine(path);
    std::string content;
    FileUtil::ReadString(file_path,content);

    v8::Isolate* isolate = setup_->isolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = setup_->context();
    context->AllowCodeGenerationFromStrings(true);
    v8::Context::Scope context_scope(context);

    //v8::Local<v8::String> resource = ScriptUtils::ToV8String(isolate, path.c_str());
    v8::Local<v8::String> resource = ScriptUtils::ToV8String(isolate, file_path.c_str());
    v8::ScriptOrigin origin(isolate, resource);
    v8::Local<v8::String> source = ScriptUtils::ToV8String(isolate, content.c_str());
    v8::TryCatch trycatch(isolate);
    v8::Local<v8::Script> compiled_script =  v8::Script::Compile(context, source, &origin).ToLocalChecked();
    if(compiled_script.IsEmpty()){
        LogWarn("compile script error");
        return;
    }
    v8::MaybeLocal<v8::Value> ret_val = compiled_script->Run(context);
    if(ret_val.IsEmpty()){
        v8::Local<v8::Value> exception = trycatch.Exception();
        //trycatch.Message()->GetScriptResourceName()
        LogWarn("execute script error", ScriptUtils::ToStdString(isolate,exception));
        return;
    }
}
void ScriptEnv::InitGlobal(){
    v8::Local<v8::Object> global = setup_->context()->Global();
    v8::Local<v8::External> env = v8::External::New(setup_->isolate(),this);
    V8Bind<&ScriptEnv::LoadModule>(setup_->isolate(),setup_->context(),global,"_toy_findModule",env);

    ObjectWrapper::WrapV8Global(setup_->isolate(),setup_->context());
}
void ScriptEnv::Stop(){
    node::Stop(setup_->env());
    setup_ = nullptr;
}

void ScriptEnv::LoadModule(const v8::FunctionCallbackInfo<v8::Value>& info){
    v8::Isolate* isolate = info.GetIsolate();
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Context::Scope context_scope(context);

    std::string require_name = ScriptUtils::ToStdString(isolate, info[0]);
    auto func_ptr = ToyEngine::Class().GetFunctionPtr(require_name);
    assert(func_ptr);
    auto obj = v8::External::New(isolate,const_cast<MetaFunction*>(func_ptr));
    auto func_wrap = v8::FunctionTemplate::New(isolate,[](const v8::FunctionCallbackInfo<v8::Value>& args){
         v8::Isolate* isolate = args.GetIsolate();
         v8::Isolate::Scope isolate_scope(isolate);
         v8::EscapableHandleScope handle_scope(isolate);
         v8::Local<v8::Context> context = isolate->GetCurrentContext();
         v8::Context::Scope context_scope(context);
        auto* meta_func =static_cast<MetaFunction*>(args.Data().As<v8::External>()->Value());
        reflect::MetaArg ret = meta_func->Invoke({&kEngine});
        //TODO: wrap metaarg to js object and return
        //1. 确定函数执行的返回值是否存在（void函数）
        //2. 确认返回值是否为class类型或者一般数值类型
        //3. 对于class类型查找MetaClass是否存在
        //4. MetaClass存在，封装MetaClass为js对象
        auto meta = ret.Meta();
        //当前meta为空表示非反射类型，以错误处理
        if(meta == nullptr){
            return;
        }

        v8::Local<v8::Object> obj = ObjectWrapper::WrapMetaArg(isolate,ret);
        args.GetReturnValue().Set(obj);
        },obj)
        ->GetFunction(context).ToLocalChecked();
        info.GetReturnValue().Set(func_wrap);
}

}