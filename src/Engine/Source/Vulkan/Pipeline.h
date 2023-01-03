#pragma once
#include "VkContext.h"

namespace toystation {
class PipelineVertexState {
public:
    void AddBindingDescription(uint32_t binding, uint32_t stride,
                               VkVertexInputRate rate);
    void AddAtributeDescription(uint32_t binding, uint32_t location,
                                VkFormat format, uint32_t offset);
    VkPipelineVertexInputStateCreateInfo* GetCreateInfo();

private:
    std::vector<VkVertexInputAttributeDescription> attrs_;
    std::vector<VkVertexInputBindingDescription> input_bindings_;
    VkPipelineVertexInputStateCreateInfo create_info_;
};
}  // namespace toystation