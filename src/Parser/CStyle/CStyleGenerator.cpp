#include "CStyleGenerator.h"
#include "Define.h"
#include "Formatter/CodeFormatter.h"

CStyleGenerator::CStyleGenerator(FileParser* parser)
    :IGenerator(parser)
{
    header_file_content.append("#pragma once \n");
    header_file_content.append("#include \"Base/Macro.h\"\n");

}
void CStyleGenerator::Generate()
{
    auto cursors = parser_->Cursors();
    if (cursors.empty())
        return;
    
    auto RelativePath = std::filesystem::relative(parser_->Path(), PathEnv::Get().ProjectFolder);
    std::string str = RelativePath.string();
    header_file_content.append("#include \"" + str + "\"\n");
    header_file_content.append("BEGIN_EXPORT\n");

    
    for (auto& cur : cursors)
    {
        if (cur->GetKind() == CXCursor_ClassDecl)
        {
            ClassCursor* cursor = static_cast<ClassCursor*>(cur.get());
            ProcessClass(cursor);
        }
    }
    info.DependCompounds = analysis.GetCompounds();
    if (!impl_file_content.empty())
    {
        header_file_content.append("END_EXPORT\n");
        std::string raw_file_name = parser_->Path().filename().replace_extension().string();
        auto RelativeFolder = std::filesystem::relative(parser_->Path().parent_path(), PathEnv::Get().ProjectFolder);
        std::filesystem::path GenFolder = PathEnv::Get().CStyleFolder;

        //save file info
        info.FileName = raw_file_name;
        info.RelativePath = RelativeFolder.string();


        GenFolder.append(RelativeFolder.string());
        if (!std::filesystem::exists(GenFolder))
        {
            std::filesystem::create_directories(GenFolder);
        }

        std::filesystem::path hfile(GenFolder);
        hfile.append(raw_file_name + "_c.h");
        
        std::ofstream hfilestream(hfile.string(), std::ios::trunc | std::ios::in);
        header_file_content = CodeFormatter::FormatCPP(header_file_content);

        hfilestream.write(header_file_content.c_str(), header_file_content.size());
        hfilestream.close();

        
        std::filesystem::path cppfile(GenFolder);
        cppfile.append(raw_file_name + "_c.cpp");
        impl_file_content = "#include \"" + raw_file_name + "_c.h" + "\"\n"+impl_file_content;

        std::ofstream cppfilestream(cppfile.string(), std::ios::trunc | std::ios::in);
        impl_file_content = CodeFormatter::FormatCPP(impl_file_content);
        cppfilestream.write(impl_file_content.c_str(), impl_file_content.size());
        cppfilestream.close();
    }
}

FileMetaInfo& CStyleGenerator::GetMetaInfo()
{
    return info;
}

void CStyleGenerator::ProcessClass(ClassCursor* cur)
{
    ClassMetaInfo meta;
    bool IsStatic = false;
    if (cur->MatchKey(SINGLETON))
    {
        IsStatic = true;
    }
    for (auto& c : cur->method_cursor)
    {
        if (c.GetAccess() == CX_CXXAccessSpecifier::CX_CXXPublic)
        {
            FuncMetaInfo func_info;
            if (!IsStatic)
            {
                ParamMetaInfo self;
                self.ParamName = "Self";
                self.Type = ParamType::POINTER;
                self.TypeName = "System.IntPtr";
                func_info.InputParams.push_back(self);
            }

            analysis.CheckResult(c, func_info);
            for (auto& arg : c.ParamCursors()) {
                analysis.CheckArgument(arg,func_info);
            }
            func_info.FuncName = cur->GetSpelling() + "_" + c.GetSpelling();
            func_info.RawName = c.GetSpelling();
            header_file_content += gen.GenerateDefine(cur, c);
            impl_file_content += gen.GenerateImpl(cur, c);
            meta.Funcs.push_back(func_info);
        }
    }
    if (!meta.Funcs.empty())
    {
        meta.ClassName = cur->GetSpelling();
        meta.NS = cur->GetNameSpace();
        meta.ClassType = IsStatic ? DeclType::STATIC : DeclType::NORMAL;
        info.Class.push_back(meta);
    }
  
}
