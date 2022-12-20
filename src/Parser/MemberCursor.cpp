#include "MemberCursor.h"

#include <iostream>

void GetParmType(Cursor c, ParamDecl& out) {
    out.name = c.GetSpelling();
    std::string start, end;

    for (auto& cur : c.GetChildren()) {
        ProcessType(cur, start, end);
    }
    out.type = start + end;
}
void ProcessType(Cursor c, std::string& start, std::string& end) {
    if (c.GetKind() == CXCursor_TypeRef) {
        start += c.GetSpelling();
    }
    if (c.GetKind() == CXCursor_NamespaceRef) {
        start += c.GetSpelling();
        start += "::";
    }
    if (c.GetKind() == CXCursor_TemplateRef) {
        start += c.GetSpelling();
        start += '<';
        end.insert(end.begin(), '>');
    }
}

MethodCursor::MethodCursor(Cursor& c, CX_CXXAccessSpecifier access)
    : Cursor(c), access_(access) {
    return_type = "void";
}

CX_CXXAccessSpecifier MethodCursor::GetAccess() { return access_; }

std::vector<std::string>& MethodCursor::Keys() { return keys; }

std::string MethodCursor::ReturnType() { return return_type; }

std::vector<ParamDecl> MethodCursor::Params() { return params; }

void MethodCursor::Process() {
    if (!IsValid()) return;

    CXType type = clang_getCursorResultType(GetCXCursor());

    return_type = ToString(clang_getTypeSpelling(type));
    int num = clang_Cursor_getNumArguments(GetCXCursor());
    std::string str;
    // FormatPrint(str);
    for (int i = 0; i < num; i++) {
        Cursor cx = clang_Cursor_getArgument(GetCXCursor(), i);
        param_cursors.push_back(cx);
        type = clang_getCursorType(cx.GetCXCursor());

        // CXType argtype = clang_getArgType(clang_getCursorType(GetCXCursor()),
        // i); std::cerr
        // <<ToString(clang_getTypeKindSpelling(argtype.kind))<<std::endl;
        std::string par_type = ToString(clang_getTypeSpelling(type));
        // std::cerr << par_type << " Kind: " <<
        // ToString(clang_getTypeKindSpelling(type.kind)) << std::endl;
        ParamDecl decl;
        decl.name = cx.GetSpelling();
        decl.type = par_type;

        params.push_back(decl);
    }
}

std::vector<Cursor> MethodCursor::ParamCursors() { return param_cursors; }

FieldCursor::FieldCursor(Cursor& c, CX_CXXAccessSpecifier access)
    : Cursor(c), access_(access) {}

CX_CXXAccessSpecifier FieldCursor::GetAccess() { return access_; }

std::string FieldCursor::DeclType() { return decl_type; }

std::vector<std::string>& FieldCursor::Keys() { return keys; }

void FieldCursor::Process() {
    CXType type = clang_getCursorType(GetCXCursor());
    decl_type = ToString(clang_getTypeSpelling(type));
    // std::cerr << decl_type << " Kind: " <<
    // ToString(clang_getTypeKindSpelling(type.kind)) << std::endl;
}
