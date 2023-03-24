//
// Created by ma on 2023/3/24.
//

#pragma once
#include <shaderc/shaderc.hpp>

namespace toystation{
//读取shader文件夹中shader.json配置文件，编译shader并缓存内容，留待渲染系统访问读取
class ShaderCompilerSystem
{

private:
    shaderc::Compiler compiler_;
};

}