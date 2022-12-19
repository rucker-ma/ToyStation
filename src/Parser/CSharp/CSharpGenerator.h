#pragma once
#include "CStyle/CStyleGenerator.h"

class CSharpGenerator
{
public:
    CSharpGenerator(CStyleGenerator* c);
    void Generate();
private:
    // �����ɵ�������ǰ���������Ľṹ��
    void GenerateCompound();
    void AddUsing(std::vector<std::string> ns);
private:
    CStyleGenerator* cstyle;
    std::string sharp_content;
    std::string bound_content;
    std::string using_segmant;
};

