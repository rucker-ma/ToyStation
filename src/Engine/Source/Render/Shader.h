#pragma once
#include <string>
#include <vulkan/vulkan.h>

namespace TSEngine {
	class Shader
	{
	public:
		Shader(std::string& file);
		VkShaderModule GetVKShaderModule();
	private:
		std::string shader_file_;
	};

}