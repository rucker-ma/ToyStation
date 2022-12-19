#pragma once
#include <string>

class CodeFormatter
{
public:
    static std::string FormatCPP(std::string source);
    static std::string FormatCS(std::string source);
};

