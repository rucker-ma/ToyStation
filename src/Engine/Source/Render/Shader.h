#pragma once

#include "Vulkan/VkContext.h"
namespace toystation {
class Shader {
public:
    Shader(std::string& file);
    VkShaderModule GetVKShaderModule();

private:
    std::string shader_file_;
};

}  // namespace toystation