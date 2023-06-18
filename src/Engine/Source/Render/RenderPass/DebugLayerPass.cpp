//
// Created by ma on 2023/6/5.
//
#include "DebugLayerPass.h"
#include "Compiler/ShaderCompilerSystem.h"
#include "Vulkan/Pipeline.h"
#include "ToyEngine.h"
namespace toystation{
bool should_render = false;
void DebugLayerPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    pipelines_.resize(SubPass_Count);
    SetupRenderPass();
    SetupDescriptorSetLayout();
    SetupPipeline();
    SetupFrameBuffer();
}
void DebugLayerPass::PostInitialize(){
    std::vector<VkWriteDescriptorSet> sets;
    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = resource_->ubo_buffer_.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);

    sets.push_back(set_container_.MakeWrite(
        0, 0, &buffer_info));
    set_container_.UpdateSets(sets);

    auto tree = kEngine.GetWorldManager().ActiveLevel()->BoundingTree();
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    outline_buffer_ = context_->GetAllocator()->CreateBuffer(cmd,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,tree->BoundingVertex());
    vertex_count_ = tree->BoundingVertex().size();

    hit_vertex_buffer_ = context_->GetAllocator()->CreateBuffer(sizeof(Vector3)*26,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    vertex_count_ = 26;

    context_->GetCommandPool()->SubmitAndWait(cmd);
//    kEngine.GetWorldManager().ActiveLevel()
}
void DebugLayerPass::Draw() {
    UpdateVertex();
    if(!should_render){
        return;
    }
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
//    updat();
    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines_[SubPass_Default].layout,0,1,
                            &set_container_.GetSet(0),0,nullptr);
    VkDeviceSize  offset = {};
    Vector3 color(1);
    vkCmdPushConstants(cmd,pipelines_[SubPass_Default].layout,VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof (Vector3),&color);
//    vkCmdBindVertexBuffers(cmd,0,1,&outline_buffer_.buffer,&offset);
//    vkCmdDraw(cmd,vertex_count_,1,0,0);
    vkCmdBindVertexBuffers(cmd,0,1,&hit_vertex_buffer_.buffer,&offset);
    vkCmdDraw(cmd,hit_vertex_count_,1,0,0);
    vkCmdEndRenderPass(cmd);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}

void DebugLayerPass::UpdateVertex(){
    auto hitresult = kEngine.GetWorldManager().ActiveLevel()->GetHitResult();
    if(!hitresult.hit){
        return;
    }
    auto hit_vertexs = hitresult.hit_mesh->BoundingBox().OutlineVertex();
    auto verts =hitresult.ray.Segment(1000);
    hit_vertexs.push_back(verts[0]);
    hit_vertexs.push_back(verts[1]);

    assert(hit_vertexs.size()==26);
    hit_vertex_count_ = hit_vertexs.size();
    should_render = true;
    void*data = context_->GetAllocator()->Map(hit_vertex_buffer_);
    memcpy(data,hit_vertexs.data(),sizeof(Vector3)*hit_vertex_count_);
    context_->GetAllocator()->UnMap(hit_vertex_buffer_);
}
void DebugLayerPass::SetupRenderPass(){
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;
    // color attachment
    VkAttachmentDescription color_attachment =
        VkHelper::DefaultAttachmentDescription(context_->GetSwapchain()->GetFormat(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,VK_ATTACHMENT_LOAD_OP_LOAD);

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
    basepass_dependency.dstSubpass = SubPass_Default;
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
    context_->GetContext()->CreateRenderPass(pass_create_info,render_pass_);
}
void DebugLayerPass::SetupDescriptorSetLayout(){
    set_container_.Init(context_->GetContext().get());
    // set =0,binding=0
    set_container_.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    set_container_.AddBinding(1,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,1,
        VK_SHADER_STAGE_FRAGMENT_BIT,0);
    descriptor_.layout = set_container_.InitLayout();

    set_container_.InitPool(1);
}
void DebugLayerPass::SetupPipeline(){
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount =  descriptor_.layout.size();
    pipeline_layout_info.pSetLayouts = descriptor_.layout.data();
    VkPushConstantRange constant_range;
    constant_range.offset =0;
    constant_range.size = sizeof(Vector3);
    constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pipeline_layout_info.pPushConstantRanges = &constant_range;
    pipeline_layout_info.pushConstantRangeCount = 1;
    context_->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SubPass_Default].layout);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages =
        VkHelper::CreateShaderState(
            context_->GetContext(),
            ShaderCompilerSystem::kCompileResult.at(kDebugPassVert),
            ShaderCompilerSystem::kCompileResult.at(kDebugtPassFrag));

    PipelineVertexState vertex_state;
    vertex_state.AddBindingDescription(0,sizeof(Vector3),VK_VERTEX_INPUT_RATE_VERTEX);
    vertex_state.AddAtributeDescription(0,0,VK_FORMAT_R32G32B32_SFLOAT,0);
//    vertex_state.AddAtributeDescription(0,1,VK_FORMAT_R32G32_SFLOAT,sizeof(Vector3));

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.topology  =VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
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
    color_blend_attachment.blendEnable = VK_TRUE; //enable blend for text output to shading texture
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
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
void DebugLayerPass::SetupFrameBuffer(){
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