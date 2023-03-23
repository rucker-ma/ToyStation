#pragma once
#include "Base/MessageQueue.h"

namespace toystation {


class Global {
public:
    static const std::thread& GetRenderThread() {
        return kRenderThreadInternal;
    }
    static void SetRenderThread(std::thread&& thread) {
        kRenderThreadInternal.swap(thread);
    }

    static const std::thread& GetTransferThread() {
        return kTransferThreadInternal;
    }
    static void SetTransferThread(std::thread&& thread) {
        kTransferThreadInternal.swap(thread);

    }
private:
    static std::thread kRenderThreadInternal;
    static std::thread kTransferThreadInternal;
};

// set after render system initialized
extern const std::thread& kRendThread;
// set after transfer system initialized
extern const std::thread& kTransferThread;

extern MessageQueue kMesssageQueue;


constexpr int kRenderMessageID = 0x0100;
constexpr int kRenderTaskID = 0x0110;
constexpr int kTransferMessageID = 0x0200;

}  // namespace toystation