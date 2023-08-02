#pragma once

#include <node/node.h>
#include <node/v8.h>

#include "Base/Vector.h"
#include "Base/Logger.h"
#include "Reflection/Meta.h"
namespace toystation{

class ScriptsSystem{
public:
    void Initialize();
    void PostInitialize();
    void Tick();
    void Clean();
private:
    void RunV8();
    void Execute(std::string filepath);
    void SetupV8Global();
private:
    SKIP_GENERATE(
    std::unique_ptr<node::MultiIsolatePlatform> platform_;
    std::unique_ptr<node::CommonEnvironmentSetup> setup_;
    )
};

}