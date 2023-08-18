#pragma once



#include "Base/Vector.h"
#include "Base/Logger.h"
#include "Meta/Meta.h"
#include "ScriptEnv.h"

namespace toystation{

class ScriptsSystem{
public:
    void Initialize();
    void PostInitialize();
    void Tick();
    void Clean();
private:
    void RunV8();
private:
    SKIP_GENERATE(
    std::unique_ptr<node::MultiIsolatePlatform> platform_;
    std::unique_ptr<ScriptEnv> env_;
    )
};

}