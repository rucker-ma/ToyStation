#pragma once
#include <fstream>
#include "code_extract.h"

class CodeGen{
public:
    CodeGen();
    void Generate(CodeExtract& extract,std::string source_relative_path);
    static std::string FieldRegister(const std::string& name,const std::string&  parent);
    static std::string FuncRegister(const std::string&  name,const std::string&  parent);
private:
    std::filesystem::path content_path_;
    std::filesystem::path header_path_;
};