#include "ToyEngine.h"
#include "ToyEngineSetting.h"

int main(int argc, char* argv[]) {

using namespace toystation;
    ToyEngineSetting::Instance().ProcessArgs(argc,argv);
    kEngine.PreInit();
    kEngine.Init();
    kEngine.PostInit();
    kEngine.Run();
    return 0;
}