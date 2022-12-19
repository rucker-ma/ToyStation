#include "CodeFormatter.h"
#include <iostream>


#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

extern "C" const char* STDCALL AStyleGetVersion(void);
extern "C" char* STDCALL AStyleMain(const char* sourceIn,
    const char* optionsIn,
    void (STDCALL * fpError)(int, const
        char*),
    char* (STDCALL * fpAlloc)(unsigned
        long));
void STDCALL ASErrorHandler(int errorNumber, const char* errorMessage)
{
    std::cerr << "astyle error " << errorNumber << "\n"
        << errorMessage << std::endl;
}
char* STDCALL ASMemoryAlloc(unsigned long memoryNeeded)
{
    char* buffer = new char[memoryNeeded];
    return buffer;
}

std::string CodeFormatter::FormatCPP(std::string source)
{
    const char* options = "--style=allman --mode=c";
    char* out = AStyleMain(source.c_str(), options, ASErrorHandler, ASMemoryAlloc);

    std::string out_str(out, strlen(out));
    delete[] out;
    return out_str;
}
std::string CodeFormatter::FormatCS(std::string source)
{
    const char* options = "--style=allman --mode=cs";
    char* out = AStyleMain(source.c_str(), options, ASErrorHandler, ASMemoryAlloc);

    std::string out_str(out, strlen(out));
    delete[] out;
    return out_str;
}
