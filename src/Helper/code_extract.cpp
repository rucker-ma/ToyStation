#include "code_extract.h"

#include <cassert>
#include <iostream>

#include "global.h"
#define COLOR_BASE 30

CodeExtract::CodeExtract(CXCursor cursor) : cursor_(cursor) {
    FindReflectDeclare();
    ExtractDeclare();
}

void CodeExtract::FindReflectDeclare() {
    clang_visitChildren(
        cursor_,
        [](CXCursor cursor, CXCursor parent, CXClientData data) {
            if (clang_Location_isFromMainFile(
                    clang_getCursorLocation(cursor)) == 0) {
                return CXChildVisit_Continue;
            }
            if (cursor.kind == CXCursor_MacroInstantiation) {
                std::string macro_name =
                    ToString(clang_getCursorDisplayName(cursor));
                if (macro_name == REFLECT_MACRO) {
                    CXToken* tokens = nullptr;
                    unsigned num_tokens = 0;
                    clang_tokenize(clang_Cursor_getTranslationUnit(parent),
                                   clang_getCursorExtent(cursor), &tokens,
                                   &num_tokens);
                    // 对于形如GENERATE_BODY(NAME)的形式，toke确定为4个，当前只取第三位为申明的反射类型
                    assert(num_tokens == 4);

                    CXString spelling = clang_getTokenSpelling(
                        clang_Cursor_getTranslationUnit(parent), tokens[2]);
                    std::string reflect_type = ToString(spelling);

                    clang_disposeTokens(clang_Cursor_getTranslationUnit(parent),
                                        tokens, num_tokens);
                    auto* candidate = static_cast<ExtractMacroPack*>(data);
                    candidate->type_names.push_back(reflect_type);
                    // candidate->push_back(cursor);
                }
//                if(macro_name == SKIP_MACRO){
//
//                }
            } else if (cursor.kind == CXCursor_ClassDecl) {
                auto* candidate = static_cast<ExtractMacroPack*>(data);
                std::string class_spelling = Spelling(cursor);
                if (std::find(candidate->type_names.begin(),
                              candidate->type_names.end(),
                              class_spelling) != candidate->type_names.end()) {
                    candidate->candidate_cursor.push_back(cursor);
                }
                // 对类中成员的遍历在后面进行，此处跳过检索
                return CXChildVisit_Continue;
            }
            return CXChildVisit_Recurse;
        },
        &macro_pack_);
}

void CodeExtract::ExtractDeclare() {
    for (auto& cursor : macro_pack_.candidate_cursor) {
        Class refl_container;
        refl_container.name = Spelling(cursor);
        clang_visitChildren(
            cursor,
            [](CXCursor cursor, CXCursor parent, CXClientData data) {
                if (clang_Location_isFromMainFile(
                        clang_getCursorLocation(cursor)) == 0) {
                    return CXChildVisit_Continue;
                }
                CXChildVisitResult visit_result = CXChildVisit_Recurse;

                //std::cout<<Spelling(cursor)<<" : " <<KindSpelling(cursor)<<std::endl;
                switch (cursor.kind) {
                    case CXCursor_FieldDecl: {
                        CXType type = clang_getCursorType(cursor);
                        std::string type_str = TypeSpelling(type);
                        std::string name_str = Spelling(cursor);
                        //std::cout << "fied decl type: " << type_str << std::endl;

                        auto* container = static_cast<Class*>(data);
                        container->properties.push_back({type_str, name_str});
                        visit_result = CXChildVisit_Continue;
                        break;
                    }
                    case CXCursor_CXXMethod:{
                        CXType result_type = clang_getCursorResultType(cursor);
                        std::string result_str = TypeSpelling(result_type);
                        int args_num = clang_Cursor_getNumArguments(cursor);
                        std::vector<MethodInput> inputs;
                        for (int i = 0; i < args_num; i++) {
                            CXCursor arg_cursor =
                                clang_Cursor_getArgument(cursor, i);
                            CXType type = clang_getCursorType(cursor);
                            std::string type_str = TypeSpelling(type);
                            std::string name_str = Spelling(cursor);
                            inputs.push_back({type_str, name_str});
                        }
                        std::string func_name = Spelling(cursor);
                        auto* container = static_cast<Class*>(data);
                        container->methods.push_back(
                            {func_name, result_str, inputs});
                        visit_result = CXChildVisit_Continue;
                        break ;
                    }
                    //这里表示反射的类中有申明结构体或其它类，需要跳过处理，避免访问到其中的成员
                    //如果类中类也标记反射，在之前反射结构查询中就被发现，不需要在这里处理
                    case CXCursor_StructDecl:
                    case CXCursor_ClassDecl:
                        visit_result = CXChildVisit_Continue;
                        break ;
                    default:
                        break ;
                }
                return visit_result;
            },
            &refl_container);
        refl_class_.push_back(refl_container);
    }
}
void CodeExtract::Print() {
    clang_visitChildren(
        cursor_,
        [](CXCursor cursor, CXCursor parent, CXClientData data) {
            if (clang_Location_isFromMainFile(
                    clang_getCursorLocation(cursor)) == 0) {
                return CXChildVisit_Continue;
            }
            auto* str_ptr = static_cast<std::string*>(data);
            str_ptr->push_back('-');
            int flag = COLOR_BASE + str_ptr->size();  // COLOR_BASE 30

            //              foreground background
            //    black        30         40
            //    red          31         41
            //    green        32         42
            //    yellow       33         43
            //    blue         34         44
            //    magenta      35         45
            //    cyan         36         46
            //    white        37         47
            std::cout << "\033[1;" << std::to_string(flag) << 'm' << *str_ptr
                      << Spelling(cursor) << "  kind:  " << KindSpelling(cursor)
                      << "\033[0m" << std::endl;

            str_ptr->pop_back();

            // return CXChildVisit_Continue;
            return CXChildVisit_Recurse;
        },
        &print_str_);
}