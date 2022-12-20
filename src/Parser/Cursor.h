#pragma once
#include "Helper.h"

class Cursor {
public:
    Cursor();
    Cursor(const CXCursor& cursor);
    bool operator==(const Cursor& cur);
    bool IsValid();
    CXCursor GetCXCursor();
    CXCursorKind GetKind();
    std::string GetKindSpelling();
    std::string GetSpelling();
    std::vector<Cursor> GetChildren();
    void FormatPrint(std::string& str);

protected:
    CXCursor cursor_;
};
