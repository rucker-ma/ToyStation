#include <filesystem>
#include <iostream>

#include "CSharp/CSharpGenerator.h"
#include "CStyle/CStyleGenerator.h"
#include "FileParser.h"

CXIndex index;

void Run(std::filesystem::directory_entry entry) {
    if (entry.status().type() == std::filesystem::file_type::directory) {
        std::filesystem::directory_iterator iter(entry);
        for (auto &i : iter) {
            if (i.status().type() == std::filesystem::file_type::regular) {
                std::filesystem::path file_path = i.path();
                if (file_path.extension().string() == ".h") {
                    FileParser fileparser(file_path.string(), index);
                    fileparser.Generate();
                    CStyleGenerator cstyle_gen(&fileparser);
                    cstyle_gen.Generate();
                }
            }
            if (i.status().type() == std::filesystem::file_type::directory) {
                Run(i);
            }
        }
    }
}
//-I include dir
//-P Project dir
//-O C Output dir
//-CS C# Output dir
bool ProcessInput(int argc, char *argv[]) {
    char res = 0;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-I") {
            while (i + 1 < argc) {
                ++i;
                if (std::string(argv[i]).at(0) == '-') {
                    --i;
                    break;
                }

                PathEnv::Get().IncludeFolder.push_back("-I" +
                                                       std::string(argv[i]));
                std::cerr << "Include Folder :" << std::string(argv[i])
                          << std::endl;
            }
            res = res ^ 0x01;
        } else if (std::string(argv[i]) == "-P") {
            ++i;
            PathEnv::Get().ProjectFolder = std::string(argv[i]);
            std::cerr << "Project Folder :" << std::string(argv[i])
                      << std::endl;
            res = res ^ 0x02;
        } else if (std::string(argv[i]) == "-O") {
            ++i;
            PathEnv::Get().CStyleFolder = std::string(argv[i]);
            std::cerr << "CStyle Folder :" << std::string(argv[i]) << std::endl;
            res = res ^ 0x04;
        }
    }
    return res == 0x0F;
}


int main(int argc, char *argv[]) {

    std::string root = "D:/project/ToyStation/src";
    std::vector<std::string> includes {
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
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.1/include"
    };
    for(auto& include_dir :includes){
        PathEnv::Get().IncludeFolder.push_back("-I"+include_dir);
    }
    PathEnv::Get().IncludeFolder.emplace_back("-std=c++20");
    PathEnv::Get().IncludeFolder.emplace_back("-DWEBRTC_WIN");
    PathEnv::Get().IncludeFolder.emplace_back("-DTOYSTATION_CUDA");
    PathEnv::Get().IncludeFolder.emplace_back("-DWEBRTC_USE_H264");
    PathEnv::Get().IncludeFolder.emplace_back("-DNOMINMAX");
    PathEnv::Get().IncludeFolder.emplace_back("-DNDEBUG");
    PathEnv::Get().ProjectFolder = root+"/Engine/Source";
    PathEnv::Get().CStyleFolder = root+"/Engine/Generated";

//    if (argc < 2) {
//        std::cerr << "Input Param Too Less" << std::endl;
//    }
//    if (!ProcessInput(argc, argv)) {
//        return 0;
//    }
    std::cout << "Receive Input Success" << std::endl;
    index = clang_createIndex(1, 1);
    if (!std::filesystem::exists(PathEnv::Get().CStyleFolder)) {
        std::filesystem::create_directories(PathEnv::Get().CStyleFolder);
    }

    std::filesystem::directory_entry source_entry(PathEnv::Get().ProjectFolder);
    Run(source_entry);

    clang_disposeIndex(index);
    return 0;
}
