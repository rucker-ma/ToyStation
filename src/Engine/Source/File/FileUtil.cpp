#include "FileUtil.h"

#include <fstream>
#include <filesystem>
#include <io.h>
#include "Base/Macro.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


namespace toystation {

constexpr int kDefaultJpgQuality = 75;

void FileUtil::ReadBinary(const std::string path, std::vector<char>& out_data) {
    ReadFile<std::vector<char>>(path,out_data);
}
void FileUtil::ReadString(const std::string path, std::string& out_string){
    ReadFile<std::string>(path,out_string);
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
    return stbi_load(name.c_str(), &width, &height, &channel,STBI_rgb_alpha);
}
std::shared_ptr<Texture> FileUtil::ReadImageAsRGBA(std::string path){
    int width =0,height=0,channel=0;
    unsigned char* data = nullptr;
    std::shared_ptr<Texture> tex = std::make_shared<Texture>();
    int data_length = 0;
    if (stbi_is_hdr(path.c_str())) {
        data = (unsigned char*)stbi_loadf(path.c_str(), &width, &height,
                                          &channel, STBI_rgb_alpha);
        tex->type = ImageType::FRAME_RGBA_F32;
        data_length = width * height * 4 * sizeof(float);
    } else {
         data =
            stbi_load(path.c_str(), &width, &height, &channel, STBI_rgb_alpha);
        tex->type = ImageType::FRAME_RGBA;
        data_length = width * height * 4 ;
    }
    assert(data && "must read data");

     tex->data = std::move(std::vector<unsigned char>(
        data, data +data_length));

    tex->width = width;
    tex->height = height;
    stbi_image_free(data);
    return tex;
}
std::string FileUtil::Combine(const char* relative_path) {
    return FileUtil::Combine(std::string(relative_path));
}
std::string FileUtil::Combine(std::string relative_path) {
//    std::filesystem::path current_path = std::filesystem::current_path();
//    auto path_str = current_path.string();
    std::string root = "D:/project/ToyStation/";
    std::string full_path = root + relative_path;
    std::ifstream f(full_path);
    if (!f.good()) {
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