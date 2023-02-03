#pragma once
#include <string>
#include <vector>

namespace toystation {
class FileUtil {
public:
    static void ReadBinary(const std::string path, std::vector<char>& out_data);
    //write 4 channel data to bmp image,just for debug
    static void WriteBmp(std::string name, unsigned char* data, int width,
                         int height);
    static unsigned char* ReadImg(std::string name,int& width,int& height,int& channel);
};
}  // namespace toystation