#include "Cursor.h"
#include <iostream>

Cursor::Cursor() {
    cursor_ = clang_getNullCursor();
}

Cursor::Cursor(const CXCursor& cursor)
    : cursor_(cursor) {
}

bool Cursor::operator==(const Cursor& cur)
{
    //return cursor_.data == cur.cursor_.data ? true : false;
    return clang_equalCursors(cursor_, cur.cursor_) != 0 ? true : false;
}

bool Cursor::IsValid()
{
    return clang_Cursor_isNull(cursor_) == 0 ? true : false;
}


CXCursor Cursor::GetCXCursor()
{
    return cursor_;
}

CXCursorKind Cursor::GetKind() {
    return clang_getCursorKind(cursor_);
}

std::string Cursor::GetKindSpelling()
{
    return ToString(clang_getCursorKindSpelling(GetKind()));
}

std::string Cursor::GetSpelling() {
    return ToString(clang_getCursorSpelling(cursor_));
}

std::vector<Cursor> Cursor::GetChildren() {
    std::vector<Cursor> children;
    
    clang_visitChildren(cursor_, [](CXCursor cursor, CXCursor parent, CXClientData data) {
    if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
    {
        return CXChildVisit_Continue;
    }
    std::vector<Cursor>* children_ptr = static_cast<std::vector<Cursor>*>(data);
    
    children_ptr->push_back(cursor);
  
    return CXChildVisit_Continue;
        }, &children);
    return children;
}

void Cursor::FormatPrint(std::string& str)
{
    clang_visitChildren(cursor_, [](CXCursor cursor, CXCursor parent, CXClientData data) {
        if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
        {
            return CXChildVisit_Continue;
        }
    std::string* str_ptr = static_cast<std::string*>(data);

    Cursor Cur = cursor;
    str_ptr->push_back('-');
    int flag = 30 + str_ptr->size();
    std::to_string(flag);    
    
    //              foreground background
    //    black        30         40
    //    red          31         41
    //    green        32         42
    //    yellow       33         43
    //    blue         34         44
    //    magenta      35         45
    //    cyan         36         46
    //    white        37         47
    std::cerr << "\033[1;"<< std::to_string(flag) <<'m' << *str_ptr << Cur.GetSpelling() << "  kind:  " << Cur.GetKindSpelling() << "\033[0m\n" << std::endl;
        Cur.FormatPrint(*str_ptr);

    str_ptr->pop_back();


    return CXChildVisit_Continue;
        }, &str);
}


