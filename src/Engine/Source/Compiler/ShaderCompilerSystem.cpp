//
// Created by ma on 2023/3/24.
//
#include "ShaderCompilerSystem.h"

#include <shaderc/status.h>

#include "Base/Logger.h"
#include "File/FileUtil.h"
#include "Input/InputComponent.h"
#include "ToyEngine.h"

namespace toystation {
std::unordered_map<std::string, std::vector<char>>
    ShaderCompilerSystem::kCompileResult;

static const std::map<std::string, shaderc_shader_kind>
    kSuffixShaderTypeMapping{{".vert", shaderc_vertex_shader},
                             {".frag", shaderc_fragment_shader},
                             {".comp", shaderc_compute_shader}};

class ShaderIncludeUtil:public shaderc::CompileOptions::IncluderInterface{
public:
    struct ShaderIncludePacket{
        shaderc_include_result result;
        std::vector<char> data;
        std::string path;
    };

    shaderc_include_result* GetInclude(const char* requested_source,
                                       shaderc_include_type type,
                                       const char* requesting_source,
                                       size_t include_depth)override{
//        FileUtil::ReadBinary(file_path.path, data);
        assert(type == shaderc_include_type::shaderc_include_type_relative);

        std::shared_ptr<ShaderIncludePacket> packet =std::make_shared<ShaderIncludePacket>();

        std::string relative_path = "src/Engine/Shader/"+std::string(requested_source);
        packet->path = FileUtil::Combine(relative_path.c_str());
        LogInfo("get include shader : " + relative_path);
        FileUtil::ReadBinary(packet->path, packet->data);

        if(packet->data.empty()){
            packet->result.source_name = nullptr;
            LogWarn("load include shader failed");
        }else{
            packet->result.source_name = packet->path.c_str();
            packet->result.source_name_length = packet->path.length();
            packet->result.content = packet->data.data();
            packet->result.content_length = packet->data.size();
        }
        packets_.push_back(packet);
        return &packet->result;
    }
    void ReleaseInclude(shaderc_include_result* data)override{
        data->source_name = nullptr;
        data->content = nullptr;
    }

private:
    std::list<std::shared_ptr<ShaderIncludePacket>>packets_;
};
void ShaderCompilerSystem::Initialize(){
    LoadShaderConfig();
    auto response =std::make_shared<CustomInputEventResponse>();
    response->name = "shader";
    response->handler = [this](Json::Value packet){
        if(ReloadShader()){
          kEngine.PushRenderFlag(RenderAction::Render_UpdatePipeline);
        }
    };
    kEngine.GetInputSystem().RegisterCustomResponse(response);
}
void ShaderCompilerSystem::LoadShaderConfig() {
    LoadShader(kCompileResult);
}
bool ShaderCompilerSystem::ReloadShader(){
    std::unordered_map<std::string,std::vector<char>> container;
    bool ret = LoadShader(container);
    if(ret){
        kCompileResult.swap(container);
    }
    return ret;
}
bool ShaderCompilerSystem::LoadShader(std::unordered_map<std::string,std::vector<char>>& container){
    assert(container.empty()&&"container should empty");
    std::string path = FileUtil::Combine("src/Engine/Shader/shader_config.json");
    JsonParseHelper parser;
    std::vector<char> data;
    FileUtil::ReadBinary(path, data);
    Json::Value value;
    if (!parser.parse(data.data(), data.size(), value)) {
        LogFatal("read level error");
    }
    assert(value["shaders"].isArray() && "read shader config from shader.json");
    Json::Value json_shaders = value["shaders"];
    for (int i = 0; i < json_shaders.size(); i++) {
        std::string shader_type = json_shaders[i]["type"].asString();
        std::string shader_path = json_shaders[i]["file"].asString();
        std::string shader_hash = json_shaders[i]["hash"].asString();
        Json::Value compile_define_array = json_shaders[i]["define"];
        shaderc::CompileOptions options;
        //        std::vector<std::string> compile_define;
        //        std::hash<std::string>()
        for (int j = 0; j < compile_define_array.size(); j++) {
            // TODO:consider key-value tpye define
            options.AddMacroDefinition(compile_define_array[j].asString());
            //            compile_define.push_back(compile_define_array[j].asString());
        }
        options.SetIncluder(std::make_unique<ShaderIncludeUtil>());
        std::vector<char> data;
        FileUtil::ReadBinary(FileUtil::Combine(shader_path.data()), data);
        LogInfo("Compiler shader : " + shader_path);
        shaderc::SpvCompilationResult result = compiler_.CompileGlslToSpv(
            data.data(), data.size(),
            kSuffixShaderTypeMapping.at(FileUtil::GetSuffix(shader_path)), "",
            options);

        if (result.GetCompilationStatus() ==
            shaderc_compilation_status::shaderc_compilation_status_success) {

            auto spv_data_begin = reinterpret_cast<char*>(
                const_cast<uint32_t*>(result.begin()));
            size_t length = (result.end()-result.begin())*sizeof(uint32_t);

            container.insert(std::make_pair(
                shader_type,
                std::vector<char>(spv_data_begin,spv_data_begin+ length)));
        } else {
            LogFatal(shader_path + " compile failed: "+ result.GetErrorMessage());
            assert(0&&"check shader");
            return false;
        }
    }
    return true;
}
}  // namespace toystation