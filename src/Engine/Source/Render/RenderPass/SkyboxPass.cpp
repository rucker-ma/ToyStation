//
// Created by ma on 2023/5/9.
//
#include "SkyboxPass.h"
#include "Vulkan/VkHelper.h"
#include "Compiler/ShaderCompilerSystem.h"

namespace toystation{

void SkyboxPass::Initialize(RenderPassInitInfo& info){
    context_ = info.context;
    resource_ = info.resource;
    pipelines_.resize(SubPass_Count);
    SetupRenderPass(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
    SetupFrameBuffer(info);
}
void SkyboxPass::PostInitialize(){
    std::vector<VkWriteDescriptorSet> sets;

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = resource_->ubo_buffer_.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);

    sets.push_back(set_container_.MakeWrite(
        0, 0, &buffer_info));
    sets.push_back(set_container_.MakeWrite(
        0, 1, &resource_->skybox_texture.descriptor));
    set_container_.UpdateSets(sets);
}
void SkyboxPass::Draw(){
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkViewport* viewport = context_->GetSwapchain()->GetViewport();
    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();

    VkRenderPassBeginInfo render_begin_info;
    ZeroVKStruct(render_begin_info, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    render_begin_info.renderPass = render_pass_;
    render_begin_info.framebuffer = framebuffer_;
    render_begin_info.renderArea = *scissor;

    vkCmdBeginRenderPass(cmd, &render_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(cmd, 0, 1, viewport);
    vkCmdSetScissor(cmd, 0, 1, scissor);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SubPass_Default].pipeline);
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines_[SubPass_Default].layout,0,1,
                            &set_container_.GetSet(0),0,nullptr);
    vkCmdDraw(cmd,36,1,0,0);
    vkCmdEndRenderPass(cmd);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}

void SkyboxPass::SetupRenderPass(RenderPassInitInfo& info){
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;
    // color attachment
    VkAttachmentDescription color_attachment =
        VkHelper::DefaultAttachmentDescription(context_->GetSwapchain()->GetFormat(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,VK_ATTACHMENT_LOAD_OP_LOAD);
    // set default depth map is VK_FORMAT_D32_SFLOAT,
    // other option is VK_FORMAT_D32_SFLOAT_S8_UINT or
    // VK_FORMAT_D24_UNORM_S8_UINT .., we should use
    // vkGetPhysicalDeviceFormatProperties get supported format and consider
    // tiling support
    VkAttachmentDescription depth_attachment =
        VkHelper::DefaultAttachmentDescription(VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,VK_ATTACHMENT_LOAD_OP_LOAD);

    attachments.push_back(color_attachment);
    attachments.push_back(depth_attachment);

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = RenderAttachRef::kRenderAttachRefColor;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    std::vector<VkAttachmentReference> color_attachment_refs;
    color_attachment_refs.push_back(color_attachment_ref);

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment =RenderAttachRef::kRenderAttachRefDepth;
    depth_attachment_ref.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //subpass for pbr render
    VkSubpassDescription base_pass{};
    base_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    base_pass.colorAttachmentCount = color_attachment_refs.size();
    base_pass.pColorAttachments = color_attachment_refs.data();
    base_pass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency basepass_dependency{};
    basepass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    basepass_dependency.dstSubpass =SubPass_Default;
    basepass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    basepass_dependency.srcAccessMask = 0;
    basepass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                       VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    basepass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dependencies.push_back(basepass_dependency);
    subpasses.push_back(base_pass);

    VkRenderPassCreateInfo pass_create_info = VkHelper::CreateRenderPassCreateInfo(
        attachments,dependencies,subpasses);
    info.context->GetContext()->CreateRenderPass(pass_create_info,
                                                 render_pass_);
}
void SkyboxPass::SetupDescriptorSetLayout(RenderPassInitInfo& info){
    set_container_.Init(
        info.context->GetContext().get());
    // set =0,binding=0
    set_container_.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    set_container_.AddBinding(
            1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
            VK_SHADER_STAGE_FRAGMENT_BIT,0);
    descriptor_.layout = set_container_.InitLayout();
    set_container_.InitPool(1);
}
void SkyboxPass::ResetPass(){
    context_->GetContext()->DestroyPipeline(pipelines_[SubPass_Default].pipeline);
    RenderPassInitInfo info = {context_,resource_};
    SetupPipeline(info);
}
void SkyboxPass::SetupPipeline(RenderPassInitInfo& info){

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount =  descriptor_.layout.size();
    pipeline_layout_info.pSetLayouts = descriptor_.layout.data();
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SubPass_Default].layout);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages =
        VkHelper::CreateShaderState(
            context_->GetContext(),
            ShaderCompilerSystem::kCompileResult.at(kSkyBoxPassVert),
            ShaderCompilerSystem::kCompileResult.at(kSkyBoxPassFrag));

    VkPipelineVertexInputStateCreateInfo vertex_state_info;
    ZeroVKStruct(vertex_state_info,
                 VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamic_state_enables = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    // 对于inline的渲染需要设置视口的一些参数，对于offline的渲染，返回为nullptr
    VkPipelineViewportStateCreateInfo view_port_state;
    ZeroVKStruct(view_port_state,
                 VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
    view_port_state.viewportCount = 1;
    view_port_state.pViewports = info.context->GetSwapchain()->GetViewport();
    view_port_state.scissorCount = 1;
    view_port_state.pScissors = info.context->GetSwapchain()->GetScissor();

    VkPipelineDynamicStateCreateInfo dynamic_state;
    ZeroVKStruct(dynamic_state,
                 VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
    dynamic_state.dynamicStateCount = dynamic_state_enables.size();
    dynamic_state.pDynamicStates = dynamic_state_enables.data();

    VkPipelineRasterizationStateCreateInfo rasterizer =
        VkHelper::DefaultRasterizationCreateInfo();

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(
        1, color_blend_attachment);

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = color_blend_attachments.size();
    color_blending.pAttachments = color_blend_attachments.data();

    VkPipelineDepthStencilStateCreateInfo depth_stencil =
        VkHelper::DefaultDepthStencilCreateInfo();

    VkGraphicsPipelineCreateInfo pipeline_info;
    ZeroVKStruct(pipeline_info,
                 VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    pipeline_info.stageCount = shader_stages.size();
    pipeline_info.pStages = shader_stages.data();
    pipeline_info.pVertexInputState = &vertex_state_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &view_port_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipelines_[SubPass_Default].layout;
    pipeline_info.renderPass = render_pass_;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.subpass = SubPass_Default;

    info.context->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipelines_[SubPass_Default].pipeline);
}
void SkyboxPass::SetupFrameBuffer(RenderPassInitInfo& info){
    std::vector<VkImageView> attachments;
    attachments.push_back(resource_->shading_texture.descriptor.imageView);
    attachments.push_back(resource_->depth_texture.descriptor.imageView);

    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();

    VkFramebufferCreateInfo create_info;
    ZeroVKStruct(create_info, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
    create_info.renderPass = render_pass_;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.width = scissor->extent.width;
    create_info.height = scissor->extent.height;
    create_info.layers = 1;

    info.context->GetContext()->CreateFramebuffer(create_info, framebuffer_);
}
}