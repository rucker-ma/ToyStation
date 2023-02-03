#include "FileUtil.h"

#include <fstream>

#include "Base/Macro.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

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
void FileUtil::WriteBmp(std::string name, unsigned char* data, int width,
                        int height) {
    stbi_write_bmp(name.c_str(), width, height, 4, data);
}
unsigned char* FileUtil::ReadImg(std::string name, int& width, int& height,
                                 int& channel) {
    return stbi_load(name.c_str(), &width, &height, &channel, STBI_rgb_alpha);
}
}  // namespace toystation