#pragma once

#include "ClassCursor.h"
#include "MemberCursor.h"

class UnitGanerator {
public:
    UnitGanerator();
    std::string GenerateDefine(ClassCursor* c, MethodCursor& m);
    std::string GenerateImpl(ClassCursor* c, MethodCursor& m);

private:
    std::string SimpleImpl(ClassCursor* c, MethodCursor& m);
    std::string SingletonImpl(ClassCursor* c, MethodCursor& m);
};
