//
// Created by ma on 2023/5/10.
//
#include "VkHelper.h"

namespace toystation{

VkAttachmentDescription VkHelper::DefaultAttachmentDescription(VkFormat format,
                  VkImageLayout final_layout,VkAttachmentLoadOp loadop){

    VkAttachmentDescription color_attachment{};
    color_attachment.format = format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = loadop;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    if(loadop == VK_ATTACHMENT_LOAD_OP_CLEAR) {
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }else{
        color_attachment.initialLayout = final_layout;
    }
    color_attachment.finalLayout = final_layout;
    return color_attachment;
}
VkRenderPassCreateInfo VkHelper::CreateRenderPassCreateInfo(
    const std::vector<VkAttachmentDescription>& attachments,
    const std::vector<VkSubpassDependency>& dependency,
    const std::vector<VkSubpassDescription>& subpass){
    VkRenderPassCreateInfo pass_create_info{};
    pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    pass_create_info.attachmentCount = attachments.size();
    pass_create_info.pAttachments = attachments.data();
    pass_create_info.subpassCount = subpass.size();
    pass_create_info.pSubpasses = subpass.data();
    pass_create_info.dependencyCount = dependency.size();
    pass_create_info.pDependencies = dependency.data();
    return pass_create_info;
}
VkPipelineLayoutCreateInfo VkHelper::CreatePipelineLayoutCreateInfo(
    const std::vector<VkDescriptorSetLayout>&layout){
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = layout.size();
    pipeline_layout_info.pSetLayouts = layout.data();
    return pipeline_layout_info;
}
std::vector<VkPipelineShaderStageCreateInfo> VkHelper::CreateShaderState(
    std::shared_ptr<VkContext> context,
    const std::vector<char>& vertex_data,const std::vector<char>& frag_data){
    VkPipelineShaderStageCreateInfo vert_shaderstage_info{};
    vert_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shaderstage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shaderstage_info.module = context->CreateShader(vertex_data);
    vert_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shaderstage_info{};
    frag_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shaderstage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    frag_shaderstage_info.module =context->CreateShader(frag_data);
    frag_shaderstage_info.pName = "main";
    return {vert_shaderstage_info,frag_shaderstage_info};
}
VkPipelineShaderStageCreateInfo VkHelper::CreateComputeShaderState(
    std::shared_ptr<VkContext> context,const std::vector<char>& comp_data){
    VkPipelineShaderStageCreateInfo compute_stage_info{};
    ZeroVKStruct(compute_stage_info,
                 VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    compute_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compute_stage_info.module =context->CreateShader(comp_data);
    compute_stage_info.pName = "main";
    return {compute_stage_info};
}
VkPipelineRasterizationStateCreateInfo VkHelper::DefaultRasterizationCreateInfo(){
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0F;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    return rasterizer;
}
VkPipelineDepthStencilStateCreateInfo VkHelper::DefaultDepthStencilCreateInfo(
    VkCompareOp compare ){
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = compare;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;
    return depth_stencil;
}
}
