#include "FileUtil.h"

#include <fstream>
#include <io.h>
#include "Base/Macro.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


namespace toystation {

constexpr int kDefaultJpgQuality = 75;

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
void FileUtil::WritePng(std::string name, unsigned char* data, int width,
                        int height) {
    stbi_write_png(name.c_str(), width, height, 4, data, width * 4);
}

void FileUtil::WriteJpg(std::string name, unsigned char* data, int width,
                        int height) {
    // jpg quality from 1(worst) to 95 (best)
    stbi_write_jpg(name.c_str(), width, height, 4, data, kDefaultJpgQuality);
}
unsigned char* FileUtil::ReadImg(std::string name, int& width, int& height,
                                 int& channel) {
    return stbi_load(name.c_str(), &width, &height, &channel, STBI_rgb_alpha);
}
std::string FileUtil::Combine(const char* relative_path) {
    std::string root = "D:/project/ToyStation/";
    std::string full_path = root+std::string(relative_path);
    std::ifstream f(full_path);
    if(!f.good()){
        assert(0);
        LogError("path not exits");
    }
    return full_path;
}
std::vector<FileUtil::FilterPath> FileUtil::FolderFilter(
    const char* relative_path, std::vector<std::string> suffixs){
    std::string folder = FileUtil::Combine(relative_path);
    std::vector<FileUtil::FilterPath> result;
#ifdef _WIN64
    struct _finddata64i32_t fileinfo;
    intptr_t hfile = _findfirst(folder.append("\\*").c_str(),&fileinfo);

    if(hfile !=-1){
        do {
            if (fileinfo.attrib & _A_SUBDIR) {
                LogInfo("read from folder: ..");
            }else{
                std::string current_path = folder + "\\"+fileinfo.name;
                std::string path_suffix = GetSuffix(current_path);
                if(!path_suffix.empty()){
                    if(std::find(suffixs.begin(), suffixs.end(), path_suffix)!=suffixs.end()){
                        result.push_back({current_path,path_suffix});
                    }
                }
            }
        } while (_findnext(hfile,&fileinfo) == 0);
        _findclose(hfile);
    }
#endif
    return result;
}
std::string FileUtil::GetSuffix(std::string path) {
    int dot_pos = path.find_last_of('.');
    if(dot_pos == std::string::npos){
        return "";
    }
    return std::string(path.begin()+dot_pos,path.end());
}
JsonParseHelper::JsonParseHelper() {
    reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
}
bool JsonParseHelper::parse(const char* data, int size, Json::Value& json) const {
    std::string error;
    if (!reader->parse(data, data + size, &json,
                       &error)) {
        LogError("json parse error: " + error);
        return false;
    }
    return true;
}


}  // namespace toystation