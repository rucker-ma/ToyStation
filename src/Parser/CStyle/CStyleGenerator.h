#pragma once
#include <filesystem>

#include "CSharp/MetaInfo.h"
#include "ReflectionGenerator.h"
#include "TypeAnalysis.h"
#include "UnitGanerator.h"

class CStyleGenerator : public IGenerator {
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
