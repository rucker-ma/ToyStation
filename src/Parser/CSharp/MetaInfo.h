#pragma once
#include <string>
#include <vector>

enum DeclType { STATIC, NORMAL };
enum ParamType { POD, STRUCT, POINTER, ENUM, OTHER };
struct ParamMetaInfo {
    std::string TypeName;
    ParamType Type;
    std::string ParamName;
    std::vector<std::string> UsingNS;
};
struct StructMetaInfo {
    std::vector<std::string> NS;
    std::vector<ParamMetaInfo> Decls;
};
struct FuncMetaInfo {
    std::string FuncName;
    std::string RawName;
    ParamMetaInfo ReturnType;
    std::vector<ParamMetaInfo> InputParams;
};
struct ClassMetaInfo {
    // class namespace
    std::vector<std::string> NS;
    std::string ClassName;
    DeclType ClassType;
    std::vector<FuncMetaInfo> Funcs;
};

class FileMetaInfo {
public:
    std::vector<ClassMetaInfo> Class;
    std::string FileName;
    std::string RelativePath;
    std::vector<std::string> DependCompounds;
};
