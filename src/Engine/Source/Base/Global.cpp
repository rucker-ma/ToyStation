#include "Global.h"

namespace toystation {
std::thread Global::kRenderThreadInternal;
std::thread Global::kTransferThreadInternal;
const std::thread& kRendThread = Global::GetRenderThread();
const std::thread& kTransferThread = Global::GetTransferThread();

MessageQueue kMesssageQueue;
}  // namespace toystation