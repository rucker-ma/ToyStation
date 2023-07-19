#pragma once
#include "global.h"
#include <clang-c/Index.h>
#include <filesystem>
#include <string>
#include <vector>
#define REFLECT_MACRO "GENERATE_BODY"

class CodeExtract {
    friend class CodeGen;
    // 存储宏解析的信息
    struct ExtractMacroPack {
        std::vector<std::string> type_names;
        std::vector<CXCursor> candidate_cursor;
    };

    struct Property {
        std::string type;
        std::string name;
    };
    struct MethodInput {
        std::string type;
        std::string name;
    };
    struct Method {
        std::string name;
        std::string return_type;
        std::vector<MethodInput> inputs;
    };
    struct Class {
        std::string name;
        std::vector<Property> properties;
        std::vector<Method> methods;
    };

public:
    CodeExtract(CXCursor cursor);
    void Print();

private:
    void FindReflectDeclare();
    void ExtractDeclare();

private:
    CXCursor cursor_;
    ExtractMacroPack macro_pack_;
    std::string print_str_;
    std::vector<Class> refl_class_;
};