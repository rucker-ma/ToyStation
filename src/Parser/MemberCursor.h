#pragma once

#include "Cursor.h"

struct ParamDecl
{
    std::string name;
    std::string type;
};

void GetParmType(Cursor c, ParamDecl& out);

void ProcessType(Cursor c, std::string& start, std::string& end);

class MethodCursor :public Cursor
{
public:
    MethodCursor(Cursor& c, CX_CXXAccessSpecifier access);
    CX_CXXAccessSpecifier GetAccess();
    std::vector<std::string>& Keys();
    std::string ReturnType();
    std::vector<ParamDecl> Params();
    void Process();
    std::vector<Cursor> ParamCursors();
private:
    CX_CXXAccessSpecifier access_;
    std::string return_type;
    std::vector<ParamDecl> params;
    std::vector<std::string> keys;
    std::vector<Cursor> param_cursors;
};

class FieldCursor :public Cursor
{
public:
    FieldCursor(Cursor& c, CX_CXXAccessSpecifier access);
    CX_CXXAccessSpecifier GetAccess();
    std::string DeclType();
    std::vector<std::string>& Keys();
    void Process();
private:
    CX_CXXAccessSpecifier access_;
    std::string decl_type;
    std::vector<std::string> keys;

};
