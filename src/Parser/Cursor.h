#pragma once
#include <vector>

#include "Helper.h"

class Cursor {
    struct VisitData {
        Cursor* parent;
        std::vector<Cursor>* children_ptr;
    };

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
    bool& HasMacro();

protected:
    CXCursor cursor_;
    std::vector<Cursor> children;
    bool has_macro_;
};
