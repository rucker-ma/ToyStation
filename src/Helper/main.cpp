#include <filesystem>
#include <iostream>
#include "global.h"
#include "code_extract.h"
#include "code_gen.h"


CXIndex index = nullptr;
std::unique_ptr< CodeGen> generator = nullptr;
std::vector<const char*> input_args{
    "-x",
    "c++",
    "-w",
    "-E",
    "-ferror-limit=0",  // 错误过多也不会停止
    "-o clangLog.txt"   // 写输出到指定文件
};

void ProcessFile(std::string file_path) {
    CXTranslationUnit unit = clang_createTranslationUnitFromSourceFile(
        index, file_path.c_str(), static_cast<int>(input_args.size()),
        input_args.data(), 0, nullptr);
    if (unit == nullptr) {
        return;
    }
    unsigned int num = clang_getNumDiagnostics(unit);
    // 代码解析有错误，一般是之前的args中参数不完整或不正确，缺少包含的头文件或者定义的宏
    if (num > 0) {
        for (unsigned int i = 0; i < num; i++) {
            CXDiagnostic diag = clang_getDiagnostic(unit, i);
            std::string err = ToString(clang_getDiagnosticCategoryText(diag));
            clang_disposeDiagnostic(diag);
        }
        clang_disposeTranslationUnit(unit);
        return;
    }

    CXCursor root_cursor = clang_getTranslationUnitCursor(unit);
    // 从ast的根部开始处理
    CodeExtract extract(root_cursor);
    std::string relative_path =
        std::filesystem::relative(file_path,Global::Instance().code_folder)
        .generic_string();
    generator->Generate(extract, relative_path);
    clang_disposeTranslationUnit(unit);
};

void StartProcess(std::filesystem::directory_entry entry) {
    if (entry.status().type() == std::filesystem::file_type::directory) {
        std::filesystem::directory_iterator subdir_iter(entry);
        for (auto& file : subdir_iter) {
            if (file.status().type() == std::filesystem::file_type::regular) {
                std::filesystem::path file_path = file.path();
                if (file_path.extension().string() == ".h") {
                    // 处理头文件
                    std::cout << "process file: " << file_path.string()
                              << std::endl;

                    ProcessFile(file_path.string());
                }
            }
            if (file.status().type() == std::filesystem::file_type::directory) {
                StartProcess(file);
            }
        }
    }
};

const std::vector<std::string> includes{
    "D:/project/ToyStation/src/ThirdParty/glm-0.9.9.8",
    "D:/project/ToyStation/src/ThirdParty/stb_image",
    "D:/project/ToyStation/src/ThirdParty/Vulkan/Include",
    "D:/project/ToyStation/src/ThirdParty/spdlog/include",
    "D:/project/ToyStation/src/ThirdParty/vk_extension",
    "E:/WebRTC/webrtc-checkout/src",
    "E:/WebRTC/webrtc-checkout/src/third_party/abseil-cpp",
    "E:/WebRTC/webrtc-checkout/src/third_party/jsoncpp/source/include",
    "D:/project/ToyStation/src/ThirdParty/asio-1.18.2/include",
    "D:/project/ToyStation/src/ThirdParty/ffmpeg-5.1.2/include",
    "D:/project/ToyStation/src/ThirdParty/websocketpp-0.8.2",
    "D:/project/ToyStation/src/ThirdParty/freetype-2.13/include",
    "D:/project/ToyStation/src/ThirdParty/tinygltf-2.8.3",
    "D:/project/ToyStation/src/ThirdParty/renderdoc",
    "D:/project/ToyStation/src/ThirdParty/Video_Codec_SDK_12.0.16/Interface",
    "D:/project/ToyStation/src/ThirdParty/KTX-Software/include",
    "D:/project/ToyStation/src/ThirdParty/node-v18.16.1/include",
    "D:/project/ToyStation/src/Engine/Source",
    "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/include"};

// 添加这些宏libclang才能正确解析
const std::vector<std::string> defines{"-std=c++20",        "-DWEBRTC_WIN",
                                       "-DTOYSTATION_CUDA", "-DWEBRTC_USE_H264",
                                       "-DNOMINMAX",        "-DNDEBUG"};
int main(int argc, char* argv[]) {
    std::string root = "D:/project/ToyStation/src";

    for (const auto& include_dir : includes) {
        Global::Instance().command_args.push_back("-I" + include_dir);
    }
    for (const auto& define : defines) {
        Global::Instance().command_args.push_back(define);
    }
    Global::Instance().code_folder = root + "/Engine/Source";
    std::string generate_folder = root + "/Engine/Generated";
    Global::Instance().gen_folder = generate_folder;

    if (!std::filesystem::exists(generate_folder)) {
        std::filesystem::create_directories(generate_folder);
    }
    for (auto& argument : Global::Instance().command_args) {
        input_args.push_back(argument.c_str());
    }
   generator = std::make_unique<CodeGen>();
    index = clang_createIndex(1, 1);
    std::filesystem::directory_entry source_entry(
        Global::Instance().code_folder);
    StartProcess(source_entry);
    clang_disposeIndex(index);
    return 0;
}