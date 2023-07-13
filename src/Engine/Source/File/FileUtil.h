#pragma once
#include <json/json.h>

#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include "Framework/Component/MaterialComponent.h"

namespace toystation {
class FileUtil {
public:
    struct FilterPath{
        std::string path;
        std::string suffix;
    };
    static void ReadBinary(const std::string path, std::vector<char>& out_data);
    static void ReadString(const std::string path, std::string& out_string);
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
private:
    template <class T>
    static void ReadFile(const std::string path,T& out){
        LogDebug("Start Read " + path);
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            LogError("Read File open failed, file not exist or not permission");
            return;
        }
        size_t file_size = file.tellg();
        out.resize(file_size);
        file.seekg(0);
        file.read(out.data(), file_size);
        file.close();
        LogDebug("Read " + path + "Success, Size is " + std::to_string(file_size));
    }

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