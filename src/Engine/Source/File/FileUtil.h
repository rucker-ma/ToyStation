#pragma once
#include <json/json.h>

#include <memory>
#include <string>
#include <vector>
#include "Framework/Component/MaterialComponent.h"

namespace toystation {
class FileUtil {
public:
    struct FilterPath{
        std::string path;
        std::string suffix;
    };
    static void ReadBinary(const std::string path, std::vector<char>& out_data);
    // write 4 channel data to bmp image,just for debug
    static void WriteBmp(std::string name, unsigned char* data, int width,
                         int height);
    static void WritePng(std::string name, unsigned char* data, int width,
                         int height);
    static void WriteJpg(std::string name, unsigned char* data, int width,
                         int height);
    static unsigned char* ReadImg(std::string name, int& width, int& height,
                                  int& channel);
    static std::shared_ptr<Texture> ReadImageAsRGBA(std::string path);
    static std::string Combine(const char* relative_path);
    static std::string Combine(std::string relative_path);
    static std::string GetSuffix(std::string path);
    static std::vector<FilterPath> FolderFilter(
        const char* relative_path,  std::vector<std::string> suffixs);
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