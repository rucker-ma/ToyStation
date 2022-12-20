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
                    CSharpGenerator csharp_gen(&cstyle_gen);
                    csharp_gen.Generate();
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
        } else if (std::string(argv[i]) == "-CS") {
            ++i;
            PathEnv::Get().CSharpFolder = std::string(argv[i]);
            std::cerr << "CSharp Folder :" << std::string(argv[i]) << std::endl;
            res = res ^ 0x08;
        }
    }
    return res == 0x0F;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Input Param Too Less" << std::endl;
    }
    if (!ProcessInput(argc, argv)) {
        return 0;
    }
    std::cerr << "Receive Input Success" << std::endl;
    index = clang_createIndex(1, 1);
    // PathEnv::Get().ProjectFolder =
    // "D:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine";

    // PathEnv::Get().CStyleFolder = PathEnv::Get().ProjectFolder;
    // PathEnv::Get().CSharpFolder =
    // "D:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine.Net";
    // PathEnv::Get().CStyleFolder.append("Generated");

    if (!std::filesystem::exists(PathEnv::Get().CStyleFolder)) {
        std::filesystem::create_directories(PathEnv::Get().CStyleFolder);
    }

    std::filesystem::directory_entry source_entry(PathEnv::Get().ProjectFolder);
    Run(source_entry);

    clang_disposeIndex(index);
    return 0;
}
