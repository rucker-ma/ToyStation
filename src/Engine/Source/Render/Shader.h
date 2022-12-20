#pragma once
#include <vulkan/vulkan.h>

#include <string>

namespace TSEngine {
class Shader {
public:
    Shader(std::string& file);
    VkShaderModule GetVKShaderModule();

private:
    std::string shader_file_;
};

}  // namespace TSEngine