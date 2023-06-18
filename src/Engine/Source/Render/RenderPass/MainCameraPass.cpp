#include "MainCameraPass.h"

#include "Base/Global.h"
#include "Base/Time.h"
#include "Compiler/ShaderCompilerSystem.h"
#include "File/FileUtil.h"
#include "ToyEngine.h"
#include "Vulkan/Images.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VkHelper.h"
#include "Vulkan/VkImageUtil.h"

namespace toystation {

void MainCameraPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    pipelines_.resize(SubPass_Count);

    SetupRenderPass(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
    SetupSkyboxPipeline(info);
    SetupFrameBuffer(info);
}
void MainCameraPass::PostInitialize(){
    std::vector<VkWriteDescriptorSet> sets;

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = resource_->ubo_buffer_.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);

    sets.push_back(set_container_.MakeWrite(
        0, 0, &buffer_info));
    sets.push_back(set_container_.MakeWrite(
        0, 1, &resource_->irradiance_texture.descriptor));
    sets.push_back(set_container_.MakeWrite(
        0, 2, &resource_->radiance_texture.descriptor));
    sets.push_back(set_container_.MakeWrite(
        0, 3, &resource_->brdf_texture.descriptor));
    set_container_.UpdateSets(sets);
}
void MainCameraPass::Draw() {
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkRenderPassBeginInfo render_begin_info;
    ZeroVKStruct(render_begin_info, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);

    VkViewport* viewport = context_->GetSwapchain()->GetViewport();
    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();

    render_begin_info.renderPass = render_pass_;
    render_begin_info.framebuffer =framebuffer_;
    render_begin_info.renderArea = *scissor;

    VkClearValue clear_color{};
    clear_color.color = {{0.0F, 0.0F, 0.0F, 1.0F}};
    VkClearValue clear_depth{};
    clear_depth.depthStencil = {1.0F, 0};
    std::vector<VkClearValue> clear_values(1, clear_color);
    clear_values.push_back(clear_depth);
    for(int i =0;i<resource_->gbuffers.size();i++) {
        clear_values.push_back(clear_color);
    }

    render_begin_info.clearValueCount = clear_values.size();
    render_begin_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(cmd, &render_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(cmd, 0, 1, viewport);
    vkCmdSetScissor(cmd, 0, 1, scissor);

    //
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SubPass_Default].pipeline);
    VkDeviceSize offset = {};
    //一个对象中每一份顶点或材质对应一个descriptor set,在使用时进行绑定，常规最大绑定数为4个
    
    //获取渲染对象，绑定资源绘制
    for(auto& pair:RenderSystem::kRenderGlobalData.render_resource->render_objects_){
        std::shared_ptr<RenderMaterial> material;

        for(auto& mesh:pair.second->Meshes()){
            UpdateUniform(mesh);
            //TODO:bind vertex data,confirm binding correct
            vkCmdBindVertexBuffers(cmd,0,1,&mesh->position_buffer.buffer,&offset);
            vkCmdBindVertexBuffers(cmd,1,1,&mesh->normal_buffer.buffer,&offset);
            vkCmdBindVertexBuffers(cmd,2,1,&mesh->texcoord_buffer.buffer,&offset);
            vkCmdBindVertexBuffers(cmd,3,1,&mesh->tangent_buffer.buffer,&offset);

            vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines_[SubPass_Default].layout,0,1,
                                    &set_container_.GetSet(0),0,nullptr);
            if (material != pair.second->Material(mesh->material_index)) {
                material = pair.second->Material(mesh->material_index);
                if(material->IsValid()) {
                    vkCmdBindDescriptorSets(
                        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SubPass_Default].layout,
                        1, 2, &material->set_container.GetSet(0), 0, nullptr);
                }
            }
            vkCmdBindIndexBuffer(cmd,mesh->indices_buffer.buffer,0,mesh->indices_type);
            vkCmdDrawIndexed(cmd,mesh->indices_size,1,0,0,0);
        }
    }

    vkCmdEndRenderPass(cmd);
//     submit ,wait and destroy commandbuffer
    context_->GetCommandPool()->SubmitAndWait(cmd);
    // SaveImage(); //for debug
}
void MainCameraPass::ResetPass(){
    context_->GetContext()->DestroyPipeline(pipelines_[SubPass_Default].pipeline);
    RenderPassInitInfo info = {context_,resource_};
    SetupPipeline(info);
}
void MainCameraPass::SetupRenderPass(RenderPassInitInfo& info) {
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;
    // GBuffer
    VkAttachmentDescription color_attachment = VkHelper::DefaultAttachmentDescription(
                                                   context_->GetSwapchain()->GetFormat(),
                                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    // set default depth map is VK_FORMAT_D32_SFLOAT,
    // other option is VK_FORMAT_D32_SFLOAT_S8_UINT or
    // VK_FORMAT_D24_UNORM_S8_UINT .., we should use
    // vkGetPhysicalDeviceFormatProperties get supported format and consider
    // tiling support
    VkAttachmentDescription depth_attachment = VkHelper::DefaultAttachmentDescription(
     VK_FORMAT_D32_SFLOAT,
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    attachments.push_back(color_attachment);
    attachments.push_back(depth_attachment);

//    VkAttachmentDescription gbuffer_color_attachment =
//        VkHelper::DefaultAttachmentDescription(
//        VK_FORMAT_R32G32B32A32_SFLOAT,
//        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    VkAttachmentDescription gbuffer_color_attachment =
        VkHelper::DefaultAttachmentDescription(
            VK_FORMAT_R32G32B32A32_SFLOAT,
            VK_IMAGE_LAYOUT_GENERAL);
    for (int i = 0; i <resource_->gbuffers.size() ; ++i) {
        attachments.push_back(gbuffer_color_attachment);
    }

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = RenderAttachRef::kRenderAttachRefColor;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    std::vector<VkAttachmentReference> color_attachment_refs;
    color_attachment_refs.push_back(color_attachment_ref);
    for(int i =0;i<resource_->gbuffers.size();i++){
        color_attachment_ref.attachment = RenderAttachRef::kGbuffer0 +i;
        color_attachment_refs.push_back(color_attachment_ref);
    }
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
    basepass_dependency.dstSubpass = static_cast<uint32_t>(SubPass_Default);
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
void MainCameraPass::SetupDescriptorSetLayout(RenderPassInitInfo& info) {
    set_container_.Init(
        info.context->GetContext().get());
    // set =0,binding=0
    set_container_.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    set_container_.AddBinding(
        1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
        VK_SHADER_STAGE_FRAGMENT_BIT,0);
    set_container_.AddBinding(
        2,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
        VK_SHADER_STAGE_FRAGMENT_BIT,0);
    set_container_.AddBinding(
        3,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
        VK_SHADER_STAGE_FRAGMENT_BIT,0);


    set_container_.AddBinding(
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    set_container_.AddBinding(
        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    set_container_.AddBinding(
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    set_container_.AddBinding(
        3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    //set=2 ,binding=0
    set_container_.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    descriptor_.layout =
        set_container_.InitLayout();

    set_container_.InitPool(3);

}
void MainCameraPass::SetupPipeline(RenderPassInitInfo& info) {

    VkPipelineLayoutCreateInfo pipeline_layout_info =
        VkHelper::CreatePipelineLayoutCreateInfo(descriptor_.layout);

    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SubPass_Default].layout);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages =
        VkHelper::CreateShaderState(
            context_->GetContext(),
            ShaderCompilerSystem::kCompileResult.at(kMainCameraPassVert),
            ShaderCompilerSystem::kCompileResult.at(kMainCameraPassFrag));

    PipelineVertexState vertex_state;

    vertex_state.AddBindingDescription(0, sizeof(Vector3),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    vertex_state.AddBindingDescription(1, sizeof(Vector3),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    vertex_state.AddBindingDescription(2, sizeof(Vector2),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    vertex_state.AddBindingDescription(3,sizeof(Vector3),VK_VERTEX_INPUT_RATE_VERTEX);
    //inPosition
    vertex_state.AddAtributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT,0);
    //inNormal
    vertex_state.AddAtributeDescription(1, 1, VK_FORMAT_R32G32B32_SFLOAT,0);
    //inTexCoord
    vertex_state.AddAtributeDescription(2, 2, VK_FORMAT_R32G32_SFLOAT,0);
    //inTangent
    vertex_state.AddAtributeDescription(3, 3, VK_FORMAT_R32G32B32_SFLOAT,0);

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
        resource_->gbuffers.size()+1, color_blend_attachment);

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
    pipeline_info.stageCount =  shader_stages.size();
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

    info.context->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipelines_[SubPass_Default].pipeline);
}
void MainCameraPass::SetupSkyboxPipeline(RenderPassInitInfo& info) {

}
void MainCameraPass::SetupFrameBuffer(RenderPassInitInfo& info) {
    std::vector<VkImageView> attachments;
    attachments.push_back(resource_->shading_texture.descriptor.imageView);
    attachments.push_back(resource_->depth_texture.descriptor.imageView);

    for(auto& gbuffer:resource_->gbuffers){
        attachments.push_back(gbuffer.descriptor.imageView);
    }
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
void MainCameraPass::UpdateUniform(std::shared_ptr<RenderMesh> mesh) {
    resource_->ubo_.model = mesh->GetModel();
    void*data = context_->GetAllocator()->Map(resource_->ubo_buffer_);
    memcpy(data,&resource_->ubo_,sizeof(resource_->ubo_));
    context_->GetAllocator()->UnMap(resource_->ubo_buffer_);
}
// SaveImage for test and debug
void MainCameraPass::SaveImage() {
    VkRect2D* rect = context_->GetSwapchain()->GetScissor();
    VkDeviceSize mem_size = rect->extent.width * rect->extent.height * 4;
    RHIBuffer buf = context_->GetAllocator()->CreateBuffer(
        mem_size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer();
    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageUtil::CmdBarrierImageLayout(
        cmd, resource_->shading_texture.image,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        sub_range);
    VkBufferImageCopy copy_region{};
    copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.imageExtent = {rect->extent.width, rect->extent.height, 1};
    vkCmdCopyImageToBuffer(
        cmd, resource_->shading_texture.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf.buffer, 1, &copy_region);
    VkImageUtil::CmdBarrierImageLayout(
        cmd,  resource_->shading_texture.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        sub_range);
    context_->GetCommandPool()->SubmitAndWait(cmd);
    void* data = context_->GetAllocator()->Map(buf);

    FileUtil::WriteBmp("test.bmp", static_cast<unsigned char*>(data),
                       rect->extent.width, rect->extent.height);

    context_->GetAllocator()->UnMap(buf);
    context_->GetAllocator()->Destroy(buf);
}

}  // namespace toystation