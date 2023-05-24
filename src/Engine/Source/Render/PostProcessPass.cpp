//
// Created by ma on 2023/5/18.
//
#include "PostProcessPass.h"

#include "Compiler/ShaderCompilerSystem.h"
#include "Vulkan/Pipeline.h"

namespace toystation{

void PostProcessPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    pipelines_.resize(SubPass_Count);
    SetupRenderPass();
    SetupDescriptorSetLayout();
    SetupPipeline();
    SetupFrameBuffer();
    PostInitialize();
}
void PostProcessPass::PostInitialize(){

    auto model = std::make_shared<MoveGizmoModel>();
//    MoveGizmoModel model;
    move_coord_object_ = std::make_shared<RenderObject>(1);

    auto cmd = context_->GetCommandPool()->CreateCommandBuffer();

    for(auto& mesh:model->GetMeshes()) {
        std::shared_ptr<RenderMesh> render_mesh = move_coord_object_->CreateMesh();
        render_mesh->position_buffer = context_->GetAllocator()->CreateBuffer(
            cmd,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,mesh->Position()
            );
        render_mesh->indices_buffer = context_->GetAllocator()->CreateBuffer(
            cmd,VK_BUFFER_USAGE_INDEX_BUFFER_BIT,mesh->Indices()
        );
        render_mesh->indices_size = mesh->IndicesInfo().nums;
        render_mesh->indices_type = VK_INDEX_TYPE_UINT32;
    }
    context_->GetCommandPool()->SubmitAndWait(cmd);

    std::vector<VkWriteDescriptorSet> sets;
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = resource_->ubo_buffer_.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);


    frag_uniform_ = context_->GetAllocator()->CreateBuffer(sizeof(PostProcessUniform),
                                                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkDescriptorBufferInfo frag_uniform_info;
    frag_uniform_info.buffer = frag_uniform_.buffer;
    frag_uniform_info.offset = 0;
    frag_uniform_info.range = sizeof(PostProcessUniform);

    sets.push_back(set_container_.MakeWrite(
        0, 0, &buffer_info));
    sets.push_back(set_container_.MakeWrite(
        0, 1, &frag_uniform_info));
    sets.push_back(set_container_.MakeWrite(
        0,2,&resource_->gbuffers[0].descriptor
        ));
    set_container_.UpdateSets(sets);

    PostProcessUniform uniform_data;
    auto scissor = context_->GetSwapchain()->GetScissor();
    uniform_data.color = Vector3 (1.0);
    uniform_data.size = Vector2(scissor->extent.width,scissor->extent.height);

    void*data = context_->GetAllocator()->Map(frag_uniform_);
    memcpy(data,&uniform_data,sizeof(uniform_data));
    context_->GetAllocator()->UnMap(frag_uniform_);

}
void PostProcessPass::Draw() {
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

    VkClearAttachment attachment{};
    VkClearValue depth_value{};
    depth_value.depthStencil = {1.0f,0};
    attachment.colorAttachment = RenderAttachRef::kRenderAttachRefDepth;
    attachment.clearValue = depth_value;
    attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    VkClearRect clear_rect = {*scissor,0,1};
    vkCmdClearAttachments(cmd,1,&attachment,1,&clear_rect);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SubPass_Default].pipeline);

    //TODO:更新数据到uniform buffer
    //UpdateUniform();
    VkDeviceSize offset={};
    for(auto& mesh:move_coord_object_->Meshes()) {
        vkCmdBindVertexBuffers(cmd,0,1,&mesh->position_buffer.buffer,&offset);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelines_[SubPass_Default].layout, 0, 1,
                                &set_container_.GetSet(0), 0, nullptr);

        vkCmdBindIndexBuffer(cmd,mesh->indices_buffer.buffer,0,mesh->indices_type);
        vkCmdDrawIndexed(cmd,mesh->indices_size, 1, 0, 0,0);
    }
    vkCmdEndRenderPass(cmd);
    context_->GetCommandPool()->SubmitAndWait(cmd);

}
void PostProcessPass::ResetPass(){
    context_->GetContext()->DestroyPipeline(pipelines_[SubPass_Default].pipeline);
    SetupPipeline();
}

void PostProcessPass::SetupRenderPass(){

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
    context_->GetContext()->CreateRenderPass(pass_create_info,
                                                 render_pass_);
}
void PostProcessPass::SetupDescriptorSetLayout(){
    set_container_.Init(
        context_->GetContext().get());
    // set =0,binding=0
    set_container_.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    //设置颜色信息，不使用纹理，直接使用Vector颜色
    set_container_.AddBinding(1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,
        VK_SHADER_STAGE_FRAGMENT_BIT,0);
    set_container_.AddBinding(2,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
                              VK_SHADER_STAGE_FRAGMENT_BIT,0);
    descriptor_.layout = set_container_.InitLayout();
    set_container_.InitPool(1);
}
void PostProcessPass::SetupPipeline(){
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount =  descriptor_.layout.size();
    pipeline_layout_info.pSetLayouts = descriptor_.layout.data();
    context_->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SubPass_Default].layout);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages =
        VkHelper::CreateShaderState(
            context_->GetContext(),
            ShaderCompilerSystem::kCompileResult.at(kPostProcessVert),
            ShaderCompilerSystem::kCompileResult.at(kPostProcessFrag));

    //设置顶点状态
    PipelineVertexState vertex_state;
    vertex_state.AddBindingDescription(0, sizeof(Vector3),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    vertex_state.AddAtributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT,0);


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
    view_port_state.pViewports = context_->GetSwapchain()->GetViewport();
    view_port_state.scissorCount = 1;
    view_port_state.pScissors = context_->GetSwapchain()->GetScissor();

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
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_SUBTRACT;

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
    pipeline_info.pVertexInputState = vertex_state.GetCreateInfo();
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

    context_->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipelines_[SubPass_Default].pipeline);
}
void PostProcessPass::SetupFrameBuffer(){
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

    context_->GetContext()->CreateFramebuffer(create_info, framebuffer_);
}
}