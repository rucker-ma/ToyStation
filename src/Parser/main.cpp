
#include <iostream>
#include <filesystem>

#include "FileParser.h"
#include "CStyle/CStyleGenerator.h"
#include "CSharp/CSharpGenerator.h"

CXIndex index;

void Run(std::filesystem::directory_entry entry)
{
    if (entry.status().type() == std::filesystem::file_type::directory)
    {
        std::filesystem::directory_iterator iter(entry);
        for (auto& i : iter)
        {
            if (i.status().type() == std::filesystem::file_type::regular)
            {
                std::filesystem::path file_path = i.path();
                if (file_path.extension().string() == ".h")
                {
                    FileParser fileparser(file_path.string(), index);
                    fileparser.Generate();
                    CStyleGenerator cstyle_gen(&fileparser);
                    cstyle_gen.Generate();
                    CSharpGenerator csharp_gen(&cstyle_gen);
                    csharp_gen.Generate();
                }
            }
            if (i.status().type() == std::filesystem::file_type::directory&&
                i.path().filename()!="Generated")
            {
                Run(i);
            }
        }
    }
}


int main()
{
    std::cerr << "Start Generate..." << std::endl;
    index = clang_createIndex(1, 1);
    PathEnv::Get().ProjectFolder = "D:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine";

    PathEnv::Get().CStyleFolder = PathEnv::Get().ProjectFolder;
    PathEnv::Get().CSharpFolder = "D:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine.Net";
    PathEnv::Get().CStyleFolder.append("Generated");
    //PathEnv::Get().CSharpFolder.append("Sharp");

    if (!std::filesystem::exists(PathEnv::Get().CStyleFolder))
    {
        std::filesystem::create_directories(PathEnv::Get().CStyleFolder);
    }
   
    std::filesystem::directory_entry source_entry(PathEnv::Get().ProjectFolder);
    Run(source_entry);

    clang_disposeIndex(index);
    return 0;
}
