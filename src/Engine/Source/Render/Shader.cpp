#include "Shader.h"

#include <fstream>
#include <vector>

#include "VulkanContext.h"

namespace TSEngine {

std::vector<char> read_file(const std::string filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file");
    }
    size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}

Shader::Shader(std::string& file) : shader_file_(file) {}
VkShaderModule Shader::GetVKShaderModule() {
    std::vector<char> shader_code = read_file(shader_file_);
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = shader_code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(shader_code.data());
    VkShaderModule shader_module;

    if (vkCreateShaderModule(VulkanContext::Instance().Device(), &create_info,
                             nullptr, &shader_module) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }
    return shader_module;
}
}  // namespace TSEngine