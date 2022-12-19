#pragma once
#include <filesystem>
#include "ReflectionGenerator.h"
#include "UnitGanerator.h"
#include "TypeAnalysis.h"
#include "CSharp/MetaInfo.h"


class CStyleGenerator:public IGenerator
{
public:
    CStyleGenerator(FileParser* parser);
    void Generate();
    FileMetaInfo& GetMetaInfo();
private:
    void ProcessClass(ClassCursor* cur);
private:
    std::string header_file_content;
    std::string impl_file_content;
    UnitGanerator gen;
    TypeAnalysis analysis;
    FileMetaInfo info;
};

