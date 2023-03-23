#pragma once
#include <string>
#include <vector>
#include <memory>
#include <json/json.h>

namespace toystation {
class FileUtil {
public:
    static void ReadBinary(const std::string path, std::vector<char>& out_data);
    //write 4 channel data to bmp image,just for debug
    static void WriteBmp(std::string name, unsigned char* data, int width,
                         int height);
    static void WritePng(std::string name, unsigned char* data, int width,
                         int height);
    static void WriteJpg(std::string name, unsigned char* data, int width,
                         int height);
    static unsigned char* ReadImg(std::string name,int& width,int& height,int& channel);

    static std::string Combine(const char* relative_path);
};

class JsonParseHelper {
public:
    JsonParseHelper();
    bool parse(const char* data, int size, Json::Value& json) const;
private:
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader;
};
}  // namespace toystation