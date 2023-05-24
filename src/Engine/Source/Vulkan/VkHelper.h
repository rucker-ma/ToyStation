#pragma once
#include "VkContext.h"
namespace toystation {
class VkHelper{
public:
    static VkAttachmentDescription DefaultAttachmentDescription(
        VkFormat format,VkImageLayout final_layout,
        VkAttachmentLoadOp loadop = VK_ATTACHMENT_LOAD_OP_CLEAR);

    static VkRenderPassCreateInfo CreateRenderPassCreateInfo(
        const std::vector<VkAttachmentDescription>& attachments,
        const std::vector<VkSubpassDependency>& dependency,
        const std::vector<VkSubpassDescription>& subpass);
    static VkPipelineLayoutCreateInfo CreatePipelineLayoutCreateInfo(
        const std::vector<VkDescriptorSetLayout>&layout);

    static std::vector<VkPipelineShaderStageCreateInfo> CreateShaderState(
        std::shared_ptr<VkContext> context,
        const std::vector<char>& vertex_data,const std::vector<char>& frag_data);
    static VkPipelineShaderStageCreateInfo CreateComputeShaderState(
        std::shared_ptr<VkContext> context,const std::vector<char>& comp_data);
    static VkPipelineRasterizationStateCreateInfo DefaultRasterizationCreateInfo();
    static VkPipelineDepthStencilStateCreateInfo DefaultDepthStencilCreateInfo(
        VkCompareOp compare = VK_COMPARE_OP_LESS);
};
}  // namespace toystation