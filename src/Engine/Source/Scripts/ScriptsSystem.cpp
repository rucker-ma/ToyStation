#include "ScriptsSystem.h"

#include <node/uv.h>

#include "Base/Global.h"
#include "Base/Thread.h"
#include "File/FileUtil.h"
#include "ScriptUtils.h"
#include "ToyEngineSetting.h"
namespace toystation{

void ScriptsSystem::Initialize() {
    int argc = ToyEngineSetting::Instance().SysArgc();
    char** argv = ToyEngineSetting::Instance().SysArgv();
    argv = uv_setup_args(argc,argv);
    std::vector<std::string> args(argv,argv+argc);
    //enable scripts debug
    args.push_back("--inspect=127.0.0.1:9229");
    std::unique_ptr<node::InitializationResult> result =
        node::InitializeOncePerProcess(args, {node::ProcessInitializationFlags::kNoInitializeV8,
                                              node::ProcessInitializationFlags::kNoInitializeNodeV8Platform});
    if(!result->errors().empty()){
        for(auto& err:result->errors()){
            LogError("Initialize node failed: " + err);
        }
        LogError("Quit script system.");
        return;
    }
    platform_ = node::MultiIsolatePlatform::Create(4);
    v8::V8::InitializePlatform(platform_.get());
    v8::V8::Initialize();
    std::vector<std::string> errors;
    std::vector<std::string> exec_args;
    setup_ = node::CommonEnvironmentSetup::Create(platform_.get(),&errors,args,exec_args);

    Global::SetScriptThread(std::thread([this]{RunV8();}));
}
void ScriptsSystem::PostInitialize() {}
void ScriptsSystem::Tick() {}
void ScriptsSystem::Clean() {
    node::Stop(setup_->env());
    setup_ = nullptr;
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    platform_ = nullptr;
    node::TearDownOncePerProcess();
}
void ScriptsSystem::RunV8(){
    ThreadUtil::SetCurrentThreadName("node_thread");
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
    //运行入库函数
    Execute("out/dist/entry.js");
    node::SpinEventLoop(setup_->env()).FromMaybe(1);
    LogInfo("Run V8 success");
}
void ScriptsSystem::Execute(std::string filepath){
    v8::Isolate* isolate = setup_->isolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = setup_->context();
    context->AllowCodeGenerationFromStrings(true);
    v8::Context::Scope context_scope(context);

    SetupV8Global();

    std::string path = FileUtil::Combine(filepath);
    std::string content;
    FileUtil::ReadString(path,content);
    v8::Local<v8::String> resource = ScriptUtils::ToV8String(isolate, path.c_str());
    v8::ScriptOrigin origin(isolate, resource);
    v8::Local<v8::String> source = ScriptUtils::ToV8String(isolate, content.c_str());
    v8::Local<v8::Script> compiled_script =  v8::Script::Compile(context, source, &origin).ToLocalChecked();
    if(compiled_script.IsEmpty()){
        LogWarn("compile script error");
        return;
    }
    v8::MaybeLocal<v8::Value> ret_val = compiled_script->Run(context);
    if(ret_val.IsEmpty()){
        LogWarn("execute script error");
        return;
    }
}
void ConsoleLogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() > 0) {
        v8::Isolate* isolate = args.GetIsolate();
        v8::String::Utf8Value message(isolate, args[0]);
        std::string logMessage(*message);
        ScriptInfo(logMessage);
    }
}
void ScriptsSystem::SetupV8Global(){
    v8::Isolate* isolate = setup_->isolate();
    v8::Locker locker(isolate);
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = setup_->context();
    v8::Context::Scope context_scope(context);
    v8::Local<v8::Object> global = context->Global();
    global->Set(context,ScriptUtils::ToV8String(isolate,"console"),global).FromJust();
    global->Set(context,ScriptUtils::ToV8String(isolate,"log"),
                v8::FunctionTemplate::New(isolate,ConsoleLogCallback)->GetFunction(context).ToLocalChecked()).FromJust();
}
}