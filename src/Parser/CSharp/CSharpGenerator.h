#pragma once
#include "CStyle/CStyleGenerator.h"

class CSharpGenerator
{
public:
    CSharpGenerator(CStyleGenerator* c);
    void Generate();
private:
    // 在生成导出函数前生成依赖的结构体
    void GenerateCompound();
    void AddUsing(std::vector<std::string> ns);
private:
    CStyleGenerator* cstyle;
    std::string sharp_content;
    std::string bound_content;
    std::string using_segmant;
};

