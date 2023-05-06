//
// Created by ma on 2023/3/24.
//

#pragma once
#include <unordered_map>
#include <shaderc/shaderc.hpp>
#include "ShaderAlias.h"
namespace toystation{
//读取shader文件夹中shader.json配置文件，编译shader并缓存内容，留待渲染系统访问读取
class ShaderCompilerSystem
{
public:
    void Comiple();
    void LoadShaderConfig();
    bool ReloadShader();
    static std::unordered_map<std::string,std::vector<char>> kCompileResult;
private:
    bool LoadShader(std::unordered_map<std::string,std::vector<char>>& container);
private:
    shaderc::Compiler compiler_;
};

}