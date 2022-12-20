#include "ClassCursor.h"

ClassCursor::ClassCursor(Cursor& c, std::shared_ptr<MacrosDelegate> del,
                         std::vector<std::string> ns)
    : Cursor(c), del_(del), name_space(ns) {
    current_specifier = CX_CXXAccessSpecifier::CX_CXXPrivate;
}

void ClassCursor::Generate() {
    for (auto& child : GetChildren()) {
        if (child.GetKind() == CXCursor_CXXAccessSpecifier) {
            current_specifier =
                clang_getCXXAccessSpecifier(child.GetCXCursor());
        }

        if (child.GetKind() == CXCursor_CXXMethod) {
            auto method = MethodCursor(child, current_specifier);
            if (del_->Modified(method, method.Keys())) {
                method.Process();
                method_cursor.push_back(method);
            }
        }

        if (child.GetKind() == CXCursor_FieldDecl) {
            auto field = FieldCursor(child, current_specifier);
            if (del_->Modified(field, field.Keys())) {
                field.Process();
                field_cursor.push_back(field);
            }
        }
    }
}

std::string ClassCursor::GetType() {
    CXType type = clang_getCursorType(GetCXCursor());
    return ToString(clang_getTypeSpelling(type));
}

std::vector<std::string>& ClassCursor::Keys() { return keys; }

std::vector<std::string> ClassCursor::GetNameSpace() { return name_space; }

bool ClassCursor::MatchKey(std::string key) {
    auto iter = std::find(keys.begin(), keys.end(), key);
    return iter != keys.end();
}
