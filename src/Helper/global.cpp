#include "global.h"

std::string Spelling(CXCursor cursor) {
    return ToString(clang_getCursorSpelling(cursor));
}
std::string KindSpelling(CXCursor cursor){
    auto kind= clang_getCursorKind(cursor);
    return ToString(clang_getCursorKindSpelling(kind));
}
std::string TypeSpelling(CXType type){
    return ToString(clang_getTypeSpelling(type));
}
Global& Global::Instance() {
    static Global instance;
    return instance;
}