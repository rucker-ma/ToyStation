//
// Created by ma on 2023/3/8.
//

#include "Thread.h"
#ifdef _WIN32
#include <Windows.h>
#endif

#include <codecvt>

namespace toystation {

void ThreadUtil::SetCurrentThreadName(std::string name) {
#ifdef _WIN32
    std::wstring w_name =
        std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(name);
    SetThreadDescription(GetCurrentThread(), w_name.c_str());
#endif
}
}  // namespace toystation