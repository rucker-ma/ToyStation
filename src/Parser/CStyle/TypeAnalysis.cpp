#include "TypeAnalysis.h"

#include <iostream>
#include <map>

std::map<CXTypeKind, std::string> BasisType = {
    {CXType_Void, "void"},       {CXType_Bool, "bool"},
    {CXType_UChar, "byte"},      {CXType_Short, "short"},
    {CXType_UInt, "uint"},       {CXType_Int, "int"},
    {CXType_ULongLong, "ulong"}, {CXType_Double, "double"},
    {CXType_Float, "float"},     {CXType_Pointer, "System.IntPtr"}};

std::map<std::string, StructMetaInfo> CompoundType = {

};

StructMetaInfo TypeAnalysis::GetStructDef(std::string name) {
    return CompoundType[name];
}
std::vector<std::string> TypeAnalysis::GetCompounds() { return compounds; }
bool TypeAnalysis::CheckResult(MethodCursor& m, FuncMetaInfo& info) {
    CXType type = clang_getCursorResultType(m.GetCXCursor());

    CheckType(type, m, info.ReturnType);
    return false;
}

bool TypeAnalysis::CheckArgument(Cursor& c, FuncMetaInfo& info) {
    CXType type = clang_getCursorType(c.GetCXCursor());
    ParamMetaInfo param_meta;
    param_meta.ParamName = c.GetSpelling();
    CheckType(type, c, param_meta);
    info.InputParams.push_back(param_meta);
    return false;
}

void TypeAnalysis::CheckType(CXType& type, Cursor& c, ParamMetaInfo& info) {
    std::cerr << "type:" << ToString(clang_getTypeSpelling(type))
              << " kind:  " << ToString(clang_getTypeKindSpelling(type.kind))
              << std::endl;
    std::cerr << " cursor : " << c.GetSpelling()
              << " kind:  " << c.GetKindSpelling() << std::endl;
    if (type.kind == CXType_Typedef) {
        Cursor cur = clang_getTypeDeclaration(type);
        CXType under_type =
            clang_getTypedefDeclUnderlyingType(cur.GetCXCursor());
        CheckType(under_type, cur, info);
        return;
    } else if (type.kind == CXType_Record) {
        Cursor decl = clang_getTypeDeclaration(type);
        info.Type = ParamType::STRUCT;
        // info.TypeName = ToString(clang_getTypeSpelling(type));
        info.TypeName = decl.GetSpelling();
        if (CompoundType.find(info.TypeName) == CompoundType.end()) {
            compounds.push_back(info.TypeName);
            CheckRecord(type, decl);
        }
        info.UsingNS = CompoundType[info.TypeName].NS;
        return;
    } else if (type.kind == CXType_Elaborated) {
        CXType named_type = clang_Type_getNamedType(type);
        CheckType(named_type, c, info);
        return;
    }
    if (clang_isPODType(type) != 1 && type.kind != CXType_Void) {
        throw std::runtime_error("check type error,type kind");
    }

    // save normal type
    if (auto res = BasisType.find(type.kind); res != BasisType.end()) {
        if (!CheckPointer(type, info)) {
            info.Type = ParamType::POD;
            info.TypeName = res->second;
        }
        return;
    }
}

bool TypeAnalysis::CheckPointer(CXType& type, ParamMetaInfo& info) {
    if (type.kind != CXType_Pointer) return false;

    CXType pointee = clang_getPointeeType(type);
    // todo: analysis pointee type
    if (pointee.kind == CXType_Char_S) {
        info.Type = ParamType::POD;
        info.TypeName = "string";
        return true;
    }

    return false;
}

void TypeAnalysis::CheckRecord(CXType& type, Cursor& cursor) {
    StructMetaInfo Record;
    RecordClientData client_data;
    client_data.parent = this;
    client_data.record = &Record;

    // ȡ�������ռ�Ĳ㼶��ϵ
    Cursor parent = clang_getCursorSemanticParent(cursor.GetCXCursor());
    while (parent.GetKind() == CXCursor_Namespace) {
        Record.NS.push_back(parent.GetSpelling());
        parent = clang_getCursorSemanticParent(parent.GetCXCursor());
    }

    clang_Type_visitFields(
        type,
        [](CXCursor C, CXClientData client_data) -> CXVisitorResult {
            auto aly = static_cast<RecordClientData*>(client_data);
            CXType type = clang_getCursorType(C);
            Cursor member_cursor = C;
            ParamMetaInfo meta_info;
            meta_info.ParamName = ToString(clang_getCursorSpelling(C));
            aly->parent->CheckType(type, member_cursor, meta_info);
            aly->record->Decls.push_back(meta_info);

            return CXVisit_Continue;
        },
        &client_data);

    CompoundType.insert(
        std::pair<std::string, StructMetaInfo>(cursor.GetSpelling(), Record));
}
