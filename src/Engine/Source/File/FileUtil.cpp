#include "FileUtil.h"

#include <fstream>

#include "Base/Macro.h"

namespace toystation {
void FileUtil::ReadBinary(const std::string path, std::vector<char>& out_data) {
    LogDebug("Start Read " + path);
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LogError("Read File open failed, file not exist or not permission");
        return;
    }
    size_t file_size = file.tellg();
    out_data.resize(file_size);

    file.seekg(0);
    file.read(out_data.data(), file_size);
    file.close();
    LogDebug("Read " + path + "Success, Size is " + std::to_string(file_size));
}
}  // namespace toystation