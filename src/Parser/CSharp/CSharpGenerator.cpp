#include "CSharpGenerator.h"


#include "Formatter/CodeFormatter.h"
#include "Helper.h"
CSharpGenerator::CSharpGenerator(CStyleGenerator* c) : cstyle(c) {}

void CSharpGenerator::Generate() {
    FileMetaInfo& info = cstyle->GetMetaInfo();
    if (info.Class.empty()) return;
    auto csharp_folder = PathEnv::Get().CSharpFolder;
    std::string file_path =
        csharp_folder.append(info.FileName + ".gen.cs").string();

    using_segmant += "using System.Runtime.InteropServices;\n";

    GenerateCompound();

    for (auto& c : info.Class) {
        std::string prefix;
        std::string suffix;
        std::string content;
        std::string bounds;

        prefix += "namespace ToyStation \n{\n";
        suffix += "}\n";

        for (auto& ns : c.NS) {
            prefix += "namespace " + ns + "\n{\n";
            suffix += "}\n";
        }

        content += "internal static class " + c.ClassName + "Gen" + "\n{\n";
        if (c.ClassType == DeclType::STATIC) {
            bounds += "public static partial class " + c.ClassName + "\n{\n";
        } else {
            bounds += "public partial class " + c.ClassName + "\n{\n";
            // constructor
            bounds = bounds +
                     "private System.IntPtr _obj;\n \
                public " +
                     c.ClassName + "(System.IntPtr Ptr)\n{\n";
        }

        for (auto& func : c.Funcs) {
            AddUsing(func.ReturnType.UsingNS);
            content +=
                "[DllImport(\"Engine\", CharSet = CharSet.Ansi, "
                "CallingConvention = CallingConvention.Cdecl)]\n";
            content = content + "internal static extern " +
                      func.ReturnType.TypeName + " " + func.FuncName;
            content +=
                "[DllImport(\"Engine\", CharSet = CharSet.Ansi, "
                "CallingConvention = CallingConvention.Cdecl)]\n";
            content = content + "internal static extern " +
                      func.ReturnType.TypeName + " " + func.FuncName;
            std::string input = "(";
            int i = 1;
            for (auto& param : func.InputParams) {
                AddUsing(param.UsingNS);
                input = input + param.TypeName + " " + param.ParamName;
                if (i != func.InputParams.size()) {
                    input += ", ";
                }
                ++i;
            }
            input += ")";
            content = content + input + ";\n";

            input = "(";
            std::string input_param = "(";
            if (c.ClassType == DeclType::STATIC) {
                bounds = bounds + "public static " + func.ReturnType.TypeName +
                         " " + func.RawName;
                int i = 1;

                for (auto& param : func.InputParams) {
                    input_param = input_param + param.ParamName;
                    input = input + param.TypeName + " " + param.ParamName;
                    if (i != func.InputParams.size()) {
                        if (i != func.InputParams.size()) {
                            input += ", ";
                            input_param += ", ";
                        }
                        ++i;
                    }
                }
            }
            else {
                    int last = 2;
                    input_param += "_obj ";
                    if (func.InputParams.size() > 1) {
                        input_param += ", ";
                    }
                    bounds = bounds + "public " + func.ReturnType.TypeName +
                             " " + func.RawName;
                    for (size_t i = 1; i < func.InputParams.size(); i++) {
                        input_param =
                            input_param + func.InputParams[i].ParamName;
                        input = input + func.InputParams[i].TypeName + " " +
                                func.InputParams[i].ParamName;
                        if (last != func.InputParams.size()) {
                            input = input + func.InputParams[i].TypeName + " " +
                                    func.InputParams[i].ParamName;
                            if (last != func.InputParams.size()) {
                                input += ", ";
                                input_param += ", ";
                            }
                            ++last;
                        }
                    }
                    input += ")";
                    input_param += ");";
                    if (func.ReturnType.TypeName == "void") {
                        bounds = bounds + input + "\n{\n" + c.ClassName +
                                 "Gen." + func.FuncName + input_param + "\n}\n";
                    } else {
                        bounds = bounds + input + "\n{\n" + " return " +
                                 c.ClassName + "Gen." + func.FuncName +
                                 input_param + "\n}\n";
                        bounds = bounds + input + "\n{\n" + c.ClassName +
                                 "Gen." + func.FuncName + input_param + "\n}\n";
                    }
                }

                content += "\n}\n";
                bounds += "\n}\n";
                sharp_content += prefix + content + bounds + suffix;
            }

            sharp_content = using_segmant + sharp_content;
            sharp_content = CodeFormatter::FormatCS(sharp_content);
            std::ofstream sharp_stream(file_path,
                                       std::ios::trunc | std::ios::in);
            sharp_stream.write(sharp_content.c_str(), sharp_content.size());
            sharp_stream.close();
        }
    }

void CSharpGenerator::GenerateCompound() {
    FileMetaInfo& info = cstyle->GetMetaInfo();


    for (auto iter = info.DependCompounds.rbegin();
         iter != info.DependCompounds.rend(); iter++) {

        StructMetaInfo str = TypeAnalysis::GetStructDef(*iter);

        std::string prefix;
        std::string suffix;
        std::string content;

        prefix += "namespace ToyStation \n{\n";
        suffix += "}\n";


        for (auto& ns : str.NS) {
            prefix += "namespace " + ns + "\n{\n";
            suffix += "}\n";
        }

        content += "[StructLayout(LayoutKind.Sequential)]\n";
        content += "public struct " + *iter + "\n{\n";

        for (auto& del : str.Decls) {
            content += "public " + del.TypeName + " " + del.ParamName + ";\n";
        }
        content += "}\n";

        sharp_content += prefix + content + suffix;
    }
}

void CSharpGenerator::AddUsing(std::vector<std::string> ns) {
    if (ns.empty()) return;
    std::string using_decl = "using ToyStation." + ns.front();

    for (int i = 1; i < ns.size(); i++) {
        using_decl += "." + ns[i];
    }
    using_segmant += using_decl + ";\n";
}
