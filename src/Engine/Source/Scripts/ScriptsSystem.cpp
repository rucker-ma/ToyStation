#include "ScriptsSystem.h"

#include <node/uv.h>

#include "Base/Global.h"
#include "Base/Thread.h"
#include "File/FileUtil.h"
#include "ScriptUtils.h"
#include "ObjectWrapper.h"
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
    auto setup = node::CommonEnvironmentSetup::Create(platform_.get(),&errors,args,exec_args);
    env_ = std::make_unique<ScriptEnv>(std::move(setup));
    Global::SetScriptThread(std::thread([this]{RunV8();}));
}
void ScriptsSystem::PostInitialize() {}
void ScriptsSystem::Tick() {}
void ScriptsSystem::Clean() {
    if(env_){
        env_->Stop();
    }
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    platform_ = nullptr;
    node::TearDownOncePerProcess();
}
void ScriptsSystem::RunV8(){
    ThreadUtil::SetCurrentThreadName("node_thread");
    env_->Run();
    LogInfo("Run V8 success");
}
}