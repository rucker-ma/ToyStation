#pragma once
#include <filesystem>
#include <string>

#include "ClassCursor.h"
#include "Helper.h"

class FileParser {
public:
    FileParser(std::filesystem::path filepath, CXIndex index);
    ~FileParser();
    void Generate();
    void Parse(Cursor& root_cursor);
    std::vector<std::shared_ptr<Cursor>>& Cursors();
    std::filesystem::path Path();

private:
    void Verify();

private:
    std::filesystem::path file_path_;
    CXIndex idx_;
    CXTranslationUnit unit_;
    Cursor root_cursor_;
    std::vector<std::string> name_space;
    std::shared_ptr<MacrosDelegate> Del;
    std::vector<std::shared_ptr<Cursor>> output_cursors_;

    std::vector<const char*> arguments = {{
        "-x",
        "c++",         // 将输入文件视为c++语言文件
        "-std=c++17",  // 编译的语言标准
        "-DNDEBUG",
        "-w",  // 抑制警告
        // "-MG", //添加缺失的头文件到依赖文件中
        //"-M",  //只进行预处理（-E）并打印依赖头文件到控制台
        "-E",
        "-ferror-limit=0",  // 错误过多也不会停止
        // "-ID:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine",
        // //-I包含的文件夹
        // "-ID:/project/csharp/Avalonia-VK/Avalonia/src/TSEngine/Render",
        // "-ID:/software/VulkanSDK/1.3.231.1/Include",
        // "-ID:/project/cpp/graphics/vk-demo/3rd/glm-0.9.9.8",
        "-o clangLog.txt"  // 写输出到指定文件
    }};
};
