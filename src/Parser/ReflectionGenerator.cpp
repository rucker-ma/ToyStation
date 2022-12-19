#include "ReflectionGenerator.h"

IGenerator::IGenerator(FileParser* parser)
    :parser_(parser)
{
}

ReflectionGenerator::ReflectionGenerator(FileParser* parser)
    :IGenerator(parser)
{
}

void ReflectionGenerator::Generate(std::string path)
{

}
