#include "ToyEngine.h"


int main(int argc, char* argv[]) {

using namespace toystation;
    kEngine.PreInit();
    kEngine.Init();
    kEngine.PostInit();
    kEngine.Run();
return 0;
}