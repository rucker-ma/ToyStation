#pragma once
#include "MacroCursor.h"
#include "MemberCursor.h"



class ClassCursor:public Cursor
{
public:
    ClassCursor(Cursor& c, std::shared_ptr<MacrosDelegate> del,
        std::vector<std::string> ns);
    void Generate();
    std::string GetType();
    std::vector<std::string>& Keys();
    std::vector<std::string> GetNameSpace();
    bool MatchKey(std::string key);
public:

    std::vector<FieldCursor> field_cursor;
    std::vector<MethodCursor> method_cursor;

private:
    CX_CXXAccessSpecifier current_specifier;
    std::shared_ptr<MacrosDelegate> del_;
    std::vector<std::string> keys;

    std::vector<std::string> name_space;
};

