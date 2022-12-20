#pragma once
#include <clang-c/Index.h>

#include "CSharp/MetaInfo.h"
#include "MemberCursor.h"
// one file, one analysis
class TypeAnalysis {
    struct RecordClientData {
        TypeAnalysis* parent;
        StructMetaInfo* record;
    };

public:
    bool CheckResult(MethodCursor& m, FuncMetaInfo& info);
    bool CheckArgument(Cursor& c, FuncMetaInfo& info);
    void CheckType(CXType& type, Cursor& c, ParamMetaInfo& info);
    static StructMetaInfo GetStructDef(std::string name);
    std::vector<std::string> GetCompounds();

private:
    bool CheckPointer(CXType& type, ParamMetaInfo& info);

    void CheckRecord(CXType& type, Cursor& cursor);

private:
    std::vector<std::string> compounds;
};
