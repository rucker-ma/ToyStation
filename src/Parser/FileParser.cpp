#include "FileParser.h"
#include "Define.h"


FileParser::FileParser(std::filesystem::path filepath,CXIndex index)
    : file_path_(filepath), root_cursor_(),idx_(index) {
}

FileParser::~FileParser()
{
    clang_disposeTranslationUnit(unit_);
    //clang_disposeIndex(idx_);
}

void FileParser::Generate() {
    //idx_ = clang_createIndex(1, 1);
    unit_ = clang_createTranslationUnitFromSourceFile(idx_, file_path_.string().c_str(),
        arguments.size(),
        arguments.data(),
        0, nullptr);
    if (unit_ == nullptr) {
        std::runtime_error("Generate TranslationUnit Error");
    }
    Verify();
    std::cerr << "Parsing : " << std::filesystem::relative(file_path_, PathEnv::Get().ProjectFolder).string() << std::endl;
    root_cursor_ = clang_getTranslationUnitCursor(unit_);

    Del = std::make_shared<MacrosDelegate>();
    Parse(root_cursor_);
}

void FileParser::Parse(Cursor& root_cursor) {

    for (auto& child : root_cursor.GetChildren()) {
        CXCursorKind kind = child.GetKind();
        if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) 
        {
            auto  cls_cur =std::make_shared<ClassCursor>(child, Del,name_space);
            cls_cur->Generate();
            if (Del->Modified(*cls_cur,cls_cur->Keys())) {
                output_cursors_.push_back(cls_cur);
            }
        }
        if (kind == CXCursor_MacroExpansion) {
                Del->Push(child);
        }
        if (kind == CXCursor_Namespace)
        {
            name_space.push_back(child.GetSpelling());
            Parse(child);
            name_space.pop_back();
        }
    }
}

std::vector<std::shared_ptr<Cursor>>& FileParser::Cursors()
{
    return output_cursors_;
}

std::filesystem::path FileParser::Path()
{
    return file_path_;
}

void FileParser::Verify()
{
    unsigned int num = clang_getNumDiagnostics(unit_);
    for (unsigned int i = 0; i < num; i++)
    {
        CXDiagnostic diag = clang_getDiagnostic(unit_, i);
        std::string err = ToString(clang_getDiagnosticCategoryText(diag));
        clang_disposeDiagnostic(diag);
    }
}
