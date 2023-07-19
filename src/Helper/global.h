#pragma once
#include <clang-c/Index.h>
#include <filesystem>
#include <string>
#include <vector>


inline std::string ToString(const CXString& str) {
    std::string std_str = clang_getCString(str);
    clang_disposeString(str);
    return std_str;
}
std::string Spelling(CXCursor cursor);
std::string KindSpelling(CXCursor cursor);
std::string TypeSpelling(CXType type);
class Global {
public:
    std::filesystem::path code_folder;
    std::filesystem::path gen_folder;
    std::vector<std::string> command_args;

    static Global& Instance();
};