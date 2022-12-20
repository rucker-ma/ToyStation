#pragma once
#include <clang-c/Index.h>

#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

inline std::ostream& operator<<(std::ostream& stream, const CXString& str) {
    stream << clang_getCString(str);
    clang_disposeString(str);
    return stream;
}

inline std::string ToString(const CXString& str) {
    std::string std_str = clang_getCString(str);
    clang_disposeString(str);
    return std_str;
}

class PathEnv {
public:
    std::filesystem::path ProjectFolder;
    std::filesystem::path CStyleFolder;
    std::filesystem::path CSharpFolder;
    std::vector<std::string> IncludeFolder;
    static PathEnv& Get();

private:
    PathEnv() = default;
};
