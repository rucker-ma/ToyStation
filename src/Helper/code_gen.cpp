#include "code_gen.h"
#include <cassert>


CodeGen::CodeGen(){
    header_path_ = Global::Instance().gen_folder/"Gen.h";
    std::ofstream header_file_stream(header_path_);
    if(!header_file_stream.is_open()){
        assert(0&&"cannot open gen header file");
    }
    header_file_stream<<"#include \"Reflection/Meta.h\"\n";
    header_file_stream.close();
    content_path_ = Global::Instance().gen_folder/"Gen.cpp";
    std::ofstream cpp_file_stream(content_path_);
    if(!cpp_file_stream.is_open()){
        assert(0&&"cannot open gen cpp file");
    }
    cpp_file_stream<<"#include \"Gen.h\"\n";
    cpp_file_stream.close();
}
void CodeGen::Generate(CodeExtract& extract, std::string source_relative_path) {
    std::string header_str;
    std::string cpp_str;
    if(extract.refl_class_.empty()){
        return;
    }
    for (auto& refl : extract.refl_class_) {
        header_str.append("META_CLASS(" + refl.name + ");\n");
        cpp_str.append("#include \"" + source_relative_path + "\"\n");
        cpp_str.append("BEGIN_DEFINE(" + refl.name + ")\n");
        for (auto& field : refl.properties) {
            cpp_str.append(FieldRegister(field.name, refl.name));
        }
        for (auto& func : refl.methods) {
            cpp_str.append(FuncRegister(func.name, refl.name));
        }
        cpp_str.append("END_DEFINE()\n");
    }
    std::ofstream cpp_file_stream(content_path_,std::ios::app);
    cpp_file_stream<<cpp_str;
    cpp_file_stream.close();
    std::ofstream header_file_stream(header_path_,std::ios::app);
    header_file_stream<<header_str;
    header_file_stream.close();
}
std::string CodeGen::FieldRegister(const std::string& name,
                                   const std::string& parent) {
    return ".AddVariable(\"" + name + "\", &" + parent + "::" + name + ")\n";
}
std::string CodeGen::FuncRegister(const std::string& name,
                                  const std::string& parent) {
    return ".AddFunction(\"" + name + "\", &" + parent + "::" + name + ")\n";
}