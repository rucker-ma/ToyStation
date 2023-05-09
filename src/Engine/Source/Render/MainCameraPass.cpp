#include "MainCameraPass.h"

#include "Base/Global.h"
#include "Base/Time.h"
#include "Compiler/ShaderCompilerSystem.h"
#include "File/FileUtil.h"

#include "Vulkan/Images.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VkImageUtil.h"
#include "ToyEngine.h"


namespace toystation {

UniformBuffer ubo;
void MainCameraPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    gbuffers_.resize(GBUFFER_COUNT);
    SetupRenderPass(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
    SetupSkyboxPipeline(info);
    SetupFrameBuffer(info);
    PostInitialize();
}

void MainCameraPass::Draw() {
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkRenderPassBeginInfo render_begin_info;
    ZeroVKStruct(render_begin_info, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);

    VkViewport* viewport = context_->GetSwapchain()->GetViewport();
    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();

    render_begin_info.renderPass = render_pass_;
    render_begin_info.framebuffer =
        resource_->main_pass_resource_.framebuffers.front();
    render_begin_info.renderArea = *scissor;

    VkClearValue clear_color{};
    clear_color.color = {{0.0F, 0.0F, 0.0F, 1.0F}};
    VkClearValue clear_depth{};
    clear_depth.depthStencil = {1.0F, 0};
    std::vector<VkClearValue> clear_values(1, clear_color);
    clear_values.push_back(clear_depth);
    for(int i =0;i<gbuffers_.size();i++) {
        clear_values.push_back(clear_color);
    }

    render_begin_info.clearValueCount = clear_values.size();
    render_begin_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(cmd, &render_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(cmd, 0, 1, viewport);
    vkCmdSetScissor(cmd, 0, 1, scissor);

    UpdateUniform();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SUBPASS_BASEPASS].pipeline);
    VkDeviceSize offset = {};
    //一个对象中每一份顶点或材质对应一个descriptor set,在使用时进行绑定，常规最大绑定数为4个
    
    //获取渲染对象，绑定资源绘制
    for(auto& pair:RenderSystem::kRenderGlobalData.render_resource->render_objects_){
        std::shared_ptr<RenderMaterial> material;

        for(auto& mesh:pair.second->Meshes()){
            mesh->UpdateUniform(ubo);
            //TODO:bind vertex data,confirm binding correct
            vkCmdBindVertexBuffers(cmd,0,1,&mesh->position_buffer.buffer,&offset);
            vkCmdBindVertexBuffers(cmd,1,1,&mesh->normal_buffer.buffer,&offset);
            vkCmdBindVertexBuffers(cmd,2,1,&mesh->texcoord_buffer.buffer,&offset);
            vkCmdBindVertexBuffers(cmd,3,1,&mesh->tangent_buffer.buffer,&offset);

            vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines_[SUBPASS_BASEPASS].layout,0,1,
                                    &mesh->set_container.GetSet(0),0,nullptr);
            if (material != pair.second->Material(mesh->material_index)) {
                material = pair.second->Material(mesh->material_index);
                if(material->IsValid()) {
                    vkCmdBindDescriptorSets(
                        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SUBPASS_BASEPASS].layout,
                        1, 2, &material->set_container.GetSet(0), 0, nullptr);
                }
            }
            vkCmdBindIndexBuffer(cmd,mesh->indices_buffer.buffer,0,mesh->indices_type);
            vkCmdDrawIndexed(cmd,mesh->indices_size,1,0,0,0);
        }
    }
    vkCmdNextSubpass(cmd,VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines_[SUBPASS_SKYBOX].pipeline);
    vkCmdSetViewport(cmd, 0, 1, viewport);
    vkCmdSetScissor(cmd, 0, 1, scissor);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = uniform_buffer_.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);
    std::vector<VkWriteDescriptorSet> sets;
    sets.push_back(
        skybox_set_container_.MakeWrite(0, 0, &buffer_info /* uniform buffer*/));
    sets.push_back(skybox_set_container_.MakeWrite(0,1,&(resource_->skybox_texture.descriptor)));
    skybox_set_container_.UpdateSets(sets);

    ubo.has_tangent = false;
    ubo.model =glm::mat4 (1.0);
    void*data = RenderSystem::kRenderGlobalData.render_context->GetAllocator()->Map(uniform_buffer_);
    memcpy(data,&ubo,sizeof(ubo));
    RenderSystem::kRenderGlobalData.render_context->GetAllocator()->UnMap(uniform_buffer_);

    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines_[SUBPASS_SKYBOX].layout,0,1,
                            &skybox_set_container_.GetSet(0),0,nullptr);

    vkCmdDraw(cmd,36,1,0,0);//?
    vkCmdEndRenderPass(cmd);
//     submit ,wait and destroy commandbuffer
    context_->GetCommandPool()->SubmitAndWait(cmd);
    // SaveImage(); //for debug
}
void MainCameraPass::RebuildShaderAndPipeline(){
    context_->GetContext()->DestroyPipeline(pipelines_[SUBPASS_BASEPASS].pipeline);
    RenderPassInitInfo info = {context_,resource_};
    SetupPipeline(info);
}
void MainCameraPass::SetupRenderPass(RenderPassInitInfo& info) {
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;
    // GBuffer
    VkAttachmentDescription color_attachment{};
    color_attachment.format = context_->GetSwapchain()->GetFormat();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    // set default depth map is VK_FORMAT_D32_SFLOAT,
    // other option is VK_FORMAT_D32_SFLOAT_S8_UINT or
    // VK_FORMAT_D24_UNORM_S8_UINT .., we should use
    // vkGetPhysicalDeviceFormatProperties get supported format and consider
    // tiling support
    depth_attachment.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachments.push_back(color_attachment);
    attachments.push_back(depth_attachment);
    for (int i = 0; i <gbuffers_.size() ; ++i) {
        attachments.push_back(color_attachment);
    }

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = RenderAttachRef::kRenderAttachRefColor;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    std::vector<VkAttachmentReference> color_attachment_refs;
    color_attachment_refs.push_back(color_attachment_ref);
    for(int i =0;i<gbuffers_.size();i++){
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
    basepass_dependency.dstSubpass =
        static_cast<uint32_t>(MainCameraSubpassType::SUBPASS_BASEPASS);
    basepass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    basepass_dependency.srcAccessMask = 0;
    basepass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    basepass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    dependencies.push_back(basepass_dependency);
    subpasses.push_back(base_pass);

    //subpass for render skybox
    VkAttachmentReference skybox_color_attachment_ref{};
    skybox_color_attachment_ref.attachment = RenderAttachRef::kRenderAttachRefColor;
    skybox_color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription skybox_pass{};
    skybox_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    skybox_pass.colorAttachmentCount = 1;
    skybox_pass.pColorAttachments = &skybox_color_attachment_ref;
    skybox_pass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency skybox_dependency{};
    skybox_dependency.srcSubpass =  static_cast<uint32_t>(MainCameraSubpassType::SUBPASS_BASEPASS);
    skybox_dependency.dstSubpass = static_cast<uint32_t>(MainCameraSubpassType::SUBPASS_SKYBOX);
    skybox_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    skybox_dependency.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    skybox_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    skybox_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    dependencies.push_back(skybox_dependency);
    subpasses.push_back(skybox_pass);


    VkRenderPassCreateInfo pass_create_info{};
    pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    pass_create_info.attachmentCount = attachments.size();
    pass_create_info.pAttachments = attachments.data();
    pass_create_info.subpassCount = subpasses.size();
    pass_create_info.pSubpasses = subpasses.data();
    pass_create_info.dependencyCount = dependencies.size();
    pass_create_info.pDependencies = dependencies.data();

    info.context->GetContext()->CreateRenderPass(pass_create_info,
                                                 render_pass_);
    resource_->current_pass_ = render_pass_;
}
void MainCameraPass::SetupDescriptorSetLayout(RenderPassInitInfo& info) {
    resource_->main_pass_resource_.set_container.Init(
        info.context->GetContext().get());
    // set =0,binding=0
    resource_->main_pass_resource_.set_container.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0);
//    resource_->main_pass_resource_.set_container.AddBinding(
//        1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
//        VK_SHADER_STAGE_FRAGMENT_BIT,0);
    // set=1,binding=0
    resource_->main_pass_resource_.set_container.AddBinding(
        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    resource_->main_pass_resource_.set_container.AddBinding(
        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    resource_->main_pass_resource_.set_container.AddBinding(
        2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    resource_->main_pass_resource_.set_container.AddBinding(
        3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    //set=2 ,binding=0
    resource_->main_pass_resource_.set_container.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT, 2);
    descriptor_.layout =
        resource_->main_pass_resource_.set_container.InitLayout();

    resource_->main_pass_resource_.set_container.InitPool(3);

}
void MainCameraPass::SetupPipeline(RenderPassInitInfo& info) {
    if(pipelines_.size()<SUBPASS_BASEPASS+1) {
        pipelines_.push_back({});
    }
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = descriptor_.layout.size();
    pipeline_layout_info.pSetLayouts = descriptor_.layout.data();
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SUBPASS_BASEPASS].layout);

    VkPipelineShaderStageCreateInfo vert_shaderstage_info{};
    vert_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shaderstage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vert_shaderstage_info.module = context_->GetContext()->CreateShader(
        ShaderCompilerSystem::kCompileResult.at(kMainCameraPassVert));

    vert_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shaderstage_info{};
    frag_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shaderstage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    frag_shaderstage_info.module = context_->GetContext()->CreateShader(
        ShaderCompilerSystem::kCompileResult.at(kMainCameraPassFrag));
    frag_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shaderstage_info,
                                                       frag_shaderstage_info};

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
        gbuffers_.size()+1, color_blend_attachment);

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = color_blend_attachments.size();
    color_blending.pAttachments = color_blend_attachments.data();

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipeline_info;
    ZeroVKStruct(pipeline_info,
                 VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = vertex_state.GetCreateInfo();
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &view_port_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipelines_[SUBPASS_BASEPASS].layout;
    pipeline_info.renderPass = render_pass_;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.subpass = SUBPASS_BASEPASS;

    info.context->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipelines_[SUBPASS_BASEPASS].pipeline);
}
void MainCameraPass::SetupSkyboxPipeline(RenderPassInitInfo& info) {
    if(pipelines_.size()<SUBPASS_SKYBOX+1) {
        pipelines_.push_back({});
    }

    skybox_set_container_.Init(
        info.context->GetContext().get());
    // set =0,binding=0
    skybox_set_container_.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1,
        VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    skybox_set_container_.AddBinding(
        1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1,
        VK_SHADER_STAGE_FRAGMENT_BIT,0);

    auto skybox_layout = skybox_set_container_.InitLayout();
    skybox_set_container_.InitPool(1);

    uniform_buffer_ = info.context->GetAllocator()
            ->CreateBuffer(sizeof(toystation::UniformBuffer),
                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = skybox_layout.size();
    pipeline_layout_info.pSetLayouts =skybox_layout.data();
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SUBPASS_SKYBOX].layout);

    VkPipelineShaderStageCreateInfo vert_shaderstage_info{};
    vert_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shaderstage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vert_shaderstage_info.module = context_->GetContext()->CreateShader(
        ShaderCompilerSystem::kCompileResult.at(kSkyBoxPassVert));

    vert_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shaderstage_info{};
    frag_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shaderstage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    frag_shaderstage_info.module = context_->GetContext()->CreateShader(
        ShaderCompilerSystem::kCompileResult.at(kSkyBoxPassFrag));
    frag_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shaderstage_info,
                                                       frag_shaderstage_info};

//    PipelineVertexState vertex_state;
//
//    vertex_state.AddBindingDescription(0, sizeof(Vector3),
//                                       VK_VERTEX_INPUT_RATE_VERTEX);
//    vertex_state.AddBindingDescription(1, sizeof(Vector3),
//                                       VK_VERTEX_INPUT_RATE_VERTEX);
//    vertex_state.AddBindingDescription(2, sizeof(Vector2),
//                                       VK_VERTEX_INPUT_RATE_VERTEX);
//    vertex_state.AddBindingDescription(3,sizeof(Vector3),VK_VERTEX_INPUT_RATE_VERTEX);
//    //inPosition
//    vertex_state.AddAtributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT,0);
//    //inNormal
//    vertex_state.AddAtributeDescription(1, 1, VK_FORMAT_R32G32B32_SFLOAT,0);
//    //inTexCoord
//    vertex_state.AddAtributeDescription(2, 2, VK_FORMAT_R32G32_SFLOAT,0);
//    //inTangent
//    vertex_state.AddAtributeDescription(3, 3, VK_FORMAT_R32G32B32_SFLOAT,0);
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

    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipeline_info;
    ZeroVKStruct(pipeline_info,
                 VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_state_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &view_port_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipelines_[SUBPASS_SKYBOX].layout;
    pipeline_info.renderPass = render_pass_;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.subpass = SUBPASS_SKYBOX;

    info.context->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipelines_[SUBPASS_SKYBOX].pipeline);
}
void MainCameraPass::SetupFrameBuffer(RenderPassInitInfo& info) {
    auto alloc = info.context->GetAllocator();
    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();

    VkImageCreateInfo image_create_info = MakeImage2DCreateInfo(
        scissor->extent, context_->GetSwapchain()->GetFormat(),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    VkImageCreateInfo depth_create_info =
        MakeImage2DCreateInfo(scissor->extent, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    resource_->main_pass_resource_.color_image =
        alloc->CreateImage(image_create_info);
    RHIImage depth_image = alloc->CreateImage(depth_create_info);

    VkImageViewCreateInfo color_view = MakeImage2DViewCreateInfo(
        resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM);
    VkImageViewCreateInfo depth_view = MakeImage2DViewCreateInfo(
        depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_FORMAT_D32_SFLOAT);

    RHITexture color_tex = alloc->CreateTexture(
        resource_->main_pass_resource_.color_image, color_view);
    resource_->main_pass_resource_.sampler_tex = color_tex;
    RHITexture depth_tex = alloc->CreateTexture(depth_image, depth_view);

    std::vector<VkImageView> attachments;
    attachments.push_back(color_tex.descriptor.imageView);
    attachments.push_back(depth_tex.descriptor.imageView);

    for(auto& gbuffer:gbuffers_){
        gbuffer.image =  alloc->CreateImage(image_create_info);
        VkImageViewCreateInfo gbuffer_view = MakeImage2DViewCreateInfo(
            gbuffer.image.image,
            VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM);
        gbuffer.texture = alloc->CreateTexture(
            gbuffer.image, gbuffer_view);
        attachments.push_back(gbuffer.texture.descriptor.imageView);
    }

    VkFramebufferCreateInfo create_info;
    ZeroVKStruct(create_info, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
    create_info.renderPass = render_pass_;
    create_info.attachmentCount = attachments.size();
    create_info.pAttachments = attachments.data();
    create_info.width = scissor->extent.width;
    create_info.height = scissor->extent.height;
    create_info.layers = 1;

    VkFramebuffer framebuffer = nullptr;
    info.context->GetContext()->CreateFramebuffer(create_info, framebuffer);
    resource_->main_pass_resource_.framebuffers.push_back(framebuffer);
}
void MainCameraPass::UpdateUniform() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime).count();
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
//    ubo.model = glm::mat4(1.0F);
    ubo.model[3][0] = 2.0;
    ubo.model[3][1] = 0.0;
    ubo.model[3][2] = -0.3;

    auto camera = kEngine.GetWorldManager().ActiveLevel()->GetCamera();
    auto camera_component = camera->GetComponent<CameraComponent>();
    assert(camera_component);
    ubo.view = camera_component->GetView();
    ubo.proj = camera_component->GetProjection();
    ubo.camera_position = camera_component->GetPosition();
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
        cmd, resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        sub_range);
    VkBufferImageCopy copy_region{};
    copy_region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copy_region.imageExtent = {rect->extent.width, rect->extent.height, 1};
    vkCmdCopyImageToBuffer(
        cmd, resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buf.buffer, 1, &copy_region);
    VkImageUtil::CmdBarrierImageLayout(
        cmd, resource_->main_pass_resource_.color_image.image,
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