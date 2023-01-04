#include "TransferSystem.h"

#ifdef _WIN32
# pragma comment(lib, "secur32.lib")
# pragma comment(lib, "winmm.lib")
# pragma comment(lib, "dmoguids.lib")
# pragma comment(lib, "wmcodecdspuuid.lib")
# pragma comment(lib, "msdmo.lib")
# pragma comment(lib, "Strmiids.lib")
#endif
namespace toystation {
void TransferSystem::Initialize() {
    auto factory = webrtc::CreateDefaultTaskQueueFactory();
}
}  // namespace toystation
