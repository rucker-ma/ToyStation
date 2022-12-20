#pragma once
#include "FileParser.h"

class IGenerator {
public:
    IGenerator(FileParser* parser);
    virtual void Generate(){};

protected:
    FileParser* parser_;
};

class ReflectionGenerator : public IGenerator {
public:
    ReflectionGenerator(FileParser* parser);
    void Generate(std::string path);
};
