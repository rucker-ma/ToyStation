#include "UnitGanerator.h"

#include "Define.h"

UnitGanerator::UnitGanerator() {}

std::string UnitGanerator::GenerateDefine(ClassCursor* c, MethodCursor& m) {
    std::string cls_name = c->GetSpelling();
    std::string ret_type = m.ReturnType();
    std::string func_name = m.GetSpelling();
    std::vector<ParamDecl> params = m.Params();
    std::string def =
        "TS_API " + ret_type + " " + cls_name + "_" + func_name + "(";
    if (!c->MatchKey(SINGLETON)) {
        def = def + c->GetType() + "* Self, ";
    }
    int i = 1;
    for (auto& var : params) {
        def = def + var.type + " " + var.name;
        if (i != params.size()) {
            def += ", ";
        }
        i++;
    }
    def += ");\n";
    return def;
}

std::string UnitGanerator::GenerateImpl(ClassCursor* c, MethodCursor& m) {
    if (!c->MatchKey(SINGLETON)) {
        return SimpleImpl(c, m);
    }
    return SingletonImpl(c, m);
}

std::string UnitGanerator::SimpleImpl(ClassCursor* c, MethodCursor& m) {
    std::string cls_name = c->GetSpelling();
    std::string ret_type = m.ReturnType();
    std::string func_name = m.GetSpelling();
    std::vector<ParamDecl> params = m.Params();

    std::string impl = ret_type + " " + cls_name + "_" + func_name + "(" +
                       c->GetType() + "* Self";
    std::string input = "(";
    int i = 1;
    for (auto& var : params) {
        impl = impl + ", " + var.type + " " + var.name;
        input += var.name;
        if (i != params.size()) {
            input += ", ";
        }
        ++i;
    }
    input += ");";

    impl += ")\n{\n";

    impl = impl + "	return Self->" + func_name + input + "\n}\n";
    return impl;
}

std::string UnitGanerator::SingletonImpl(ClassCursor* c, MethodCursor& m) {
    std::string cls_name = c->GetSpelling();
    std::string ret_type = m.ReturnType();
    std::string func_name = m.GetSpelling();
    std::vector<ParamDecl> params = m.Params();

    std::string impl = ret_type + " " + cls_name + "_" + func_name + "(";

    std::string input = "(";
    int i = 1;
    for (auto& var : params) {
        impl = impl + var.type + " " + var.name;
        input += var.name;
        if (i != params.size()) {
            input += ", ";
            impl += ", ";
        }
        ++i;
    }
    input += ");";

    impl += ")\n{\n";

    impl = impl + "	return " + c->GetType() + "::Instance()." + func_name +
           input + "\n}\n";
    return impl;
}
