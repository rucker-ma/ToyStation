#pragma once
#include <string>
#include <vector>

namespace toystation {
class FileUtil {
public:
    static void ReadBinary(const std::string path, std::vector<char>& out_data);
};
}  // namespace toystation