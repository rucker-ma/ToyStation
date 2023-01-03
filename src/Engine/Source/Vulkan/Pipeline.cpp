#include "Pipeline.h"

#include "Helper.h"

namespace toystation {
void PipelineVertexState::AddBindingDescription(uint32_t binding,
                                                uint32_t stride,
                                                VkVertexInputRate rate) {
    VkVertexInputBindingDescription bind_description;
    bind_description.binding = binding;
    bind_description.stride = stride;
    bind_description.inputRate = rate;
    input_bindings_.push_back(bind_description);
}
void PipelineVertexState::AddAtributeDescription(uint32_t binding,
                                                 uint32_t location,
                                                 VkFormat format,
                                                 uint32_t offset) {
    VkVertexInputAttributeDescription attribute;
    attribute.binding = binding;
    attribute.location = location;
    attribute.format = format;
    attribute.offset = offset;
    attrs_.push_back(attribute);
}
VkPipelineVertexInputStateCreateInfo* PipelineVertexState::GetCreateInfo() {
    ZeroVKStruct(create_info_,
                 VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
    create_info_.vertexBindingDescriptionCount = input_bindings_.size();
    create_info_.pVertexBindingDescriptions = input_bindings_.data();
    create_info_.vertexAttributeDescriptionCount = attrs_.size();
    create_info_.pVertexAttributeDescriptions = attrs_.data();

    return &create_info_;
}
}  // namespace toystation