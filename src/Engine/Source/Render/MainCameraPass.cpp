#include "MainCameraPass.h"

#include "Base/Global.h"
#include "File/FileUtil.h"
#include "Vulkan/Images.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VkImageUtil.h"

namespace toystation {

struct UniformBuffer {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
};

const std::vector<Vertex> kVertices = {
    {{-0.5, -0.5, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0}},
    {{0.5, -0.5, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0}},
    {{0.5, 0.5, 0.0}, {0.0, 0.0, 1.0}, {1.0, 1.0}},
    {{-0.5, 0.5, 0.0}, {1.0, 1.0, 1.0}, {0.0, 1.0}},
    {{-0.5, -0.5, -0.5}, {1.0, 0.0, 0.0}, {0.0, 0.0}},
    {{0.5, -0.5, -0.5}, {0.0, 1.0, 0.0}, {1.0, 0.0}},
    {{0.5, 0.5, -0.5}, {0.0, 0.0, 1.0}, {1.0, 1.0}},
    {{-0.5, 0.5, -0.5}, {1.0, 1.0, 1.0}, {0.0, 1.0}}};

const std::vector<uint16_t> kIndices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};

struct TestBuffer {
    Buffer vert;
    Buffer indices;
    Buffer uniform;
};

TestBuffer kShaderBuffer;

VkShaderModule GetShader(std::string path, std::shared_ptr<VkContext> ctx) {
    std::vector<char> data;
    FileUtil::ReadBinary(path, data);
    return ctx->CreateShader(data.data(), data.size());
}

void MainCameraPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    SetupRenderPass(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
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

    UpdateUniform();
    render_begin_info.renderPass = render_pass_;
    render_begin_info.framebuffer =
        resource_->main_pass_resource_.framebuffers.front();
    render_begin_info.renderArea = *scissor;

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
    clear_values[1].depthStencil = {1.0F, 0};
    render_begin_info.clearValueCount = clear_values.size();
    render_begin_info.pClearValues = clear_values.data();
    vkCmdBeginRenderPass(cmd, &render_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(cmd, 0, 1, viewport);
    vkCmdSetScissor(cmd, 0, 1, scissor);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.pipeline);
    VkDeviceSize offset = {};
    vkCmdBindVertexBuffers(cmd, 0, 1, &kShaderBuffer.vert.buffer, &offset);
    vkCmdBindDescriptorSets(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_.layout, 0, 1,
        &resource_->main_pass_resource_.set_container.GetSet(0), 0, nullptr);

    vkCmdBindIndexBuffer(cmd, kShaderBuffer.indices.buffer, 0,
                         VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, kIndices.size(), 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd);
    //submit ,wait and destroy commandbuffer


    context_->GetCommandPool()->SubmitAndWait(cmd);
    //SaveImage(); //for debug
}

void MainCameraPass::SetAttachmentResource() {}
void MainCameraPass::SetupRenderPass(RenderPassInitInfo& info) {
    // 主相机使用的颜色缓冲
    VkAttachmentDescription color_attachment{};
    color_attachment.format = info.context->GetSwapchain()->GetFormat();
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

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = RenderAttachRef::kRenderAttachRefColor;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = RenderAttachRef::kRenderAttachRefDepth;
    depth_attachment_ref.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription base_pass{};
    base_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    base_pass.colorAttachmentCount = 1;
    base_pass.pColorAttachments = &color_attachment_ref;
    base_pass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass =
        static_cast<uint32_t>(MainCameraSubpassType::SUBPASS_BASEPASS);
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDependency> dependencies;
    std::vector<VkSubpassDescription> subpasses;

    attachments.push_back(color_attachment);
    attachments.push_back(depth_attachment);
    dependencies.push_back(dependency);
    subpasses.push_back(base_pass);

    // rgb to yuv subpass,use compute shader process
    //  VkAttachmentDescription yuv420_color_attachment{};
    //  yuv420_color_attachment.format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    //  //420 3plane format yuv420_color_attachment.samples =
    //  VK_SAMPLE_COUNT_1_BIT; yuv420_color_attachment.loadOp =
    //  VK_ATTACHMENT_LOAD_OP_CLEAR; yuv420_color_attachment.storeOp =
    //  VK_ATTACHMENT_STORE_OP_STORE; yuv420_color_attachment.stencilLoadOp =
    //  VK_ATTACHMENT_LOAD_OP_DONT_CARE; yuv420_color_attachment.stencilStoreOp
    //  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //  yuv420_color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //  yuv420_color_attachment.finalLayout =
    //  VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    // VkAttachmentReference yuv420_attachment_ref{};
    // yuv420_attachment_ref.attachment =
    // RenderAttachRef::kRenderAttachRefYuv420; yuv420_attachment_ref.layout =
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // VkSubpassDependency trans_dependency{};
    // trans_dependency.srcSubpass = static_cast<uint32_t>(
    // MainCameraSubpassType::SUBPASS_BASEPASS); trans_dependency.dstSubpass =
    // static_cast<uint32_t>( MainCameraSubpassType::SUBPASS_YUV_TRANSFER);
    // trans_dependency.srcStageMask =
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
    //                           VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    // trans_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    // trans_dependency.dstStageMask =
    // VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT|
    // VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    //     trans_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    //     ;

    // VkSubpassDescription yuv_pass{};
    // yuv_pass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    // yuv_pass.colorAttachmentCount = 1;
    // yuv_pass.pColorAttachments = &yuv420_attachment_ref;

    // attachments.push_back(yuv420_color_attachment);
    // dependencies.push_back(trans_dependency);
    // subpasses.push_back(yuv_pass);

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
    resource_->main_pass_resource_.set_container.AddBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    resource_->main_pass_resource_.set_container.AddBinding(
        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
        VK_SHADER_STAGE_FRAGMENT_BIT);
    descriptor_.layout =
        resource_->main_pass_resource_.set_container.InitLayout();
    // how to set default set? now use default 2
    resource_->main_pass_resource_.set_container.InitPool(2);

    std::vector<VkWriteDescriptorSet> sets;
    kShaderBuffer.uniform = info.context->GetAllocator()->CreateBuffer(
        sizeof(toystation::UniformBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = kShaderBuffer.uniform.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);

    LoadTexture();

    sets.push_back(resource_->main_pass_resource_.set_container.MakeWrite(
        0, 0, &buffer_info /* uniform buffer*/));

    sets.push_back(resource_->main_pass_resource_.set_container.MakeWrite(
        0, 1, &image_tex_.descriptor));
    resource_->main_pass_resource_.set_container.UpdateSets(sets);

    // 加载顶点数据到显存
    kShaderBuffer.vert = info.context->GetAllocator()->CreateBuffer(
        sizeof(Vertex) * kVertices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    void* data = info.context->GetAllocator()->Map(kShaderBuffer.vert);
    memcpy(data, kVertices.data(), sizeof(Vertex) * kVertices.size());
    info.context->GetAllocator()->UnMap(kShaderBuffer.vert);
    // 加载索引数据到显存
    kShaderBuffer.indices = info.context->GetAllocator()->CreateBuffer(
        sizeof(uint16_t) * kIndices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    data = info.context->GetAllocator()->Map(kShaderBuffer.indices);
    memcpy(data, kIndices.data(), sizeof(uint16_t) * kIndices.size());
    info.context->GetAllocator()->UnMap(kShaderBuffer.indices);
}
void MainCameraPass::SetupPipeline(RenderPassInitInfo& info) {
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_.layout;
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipeline_.layout);

    VkPipelineShaderStageCreateInfo vert_shaderstage_info{};
    vert_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shaderstage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shaderstage_info.module =
        GetShader("D:/project/ToyStation/src/Engine/Shader/vert.spv",
                  info.context->GetContext());

    vert_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shaderstage_info{};
    frag_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shaderstage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shaderstage_info.module =
        GetShader("D:/project/ToyStation/src/Engine/Shader/frag.spv",
                  info.context->GetContext());
    frag_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shaderstage_info,
                                                       frag_shaderstage_info};

    PipelineVertexState vertex_state;
    vertex_state.AddBindingDescription(0, sizeof(Vertex),
                                       VK_VERTEX_INPUT_RATE_VERTEX);
    vertex_state.AddAtributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT,
                                        offsetof(Vertex, pos));
    vertex_state.AddAtributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT,
                                        offsetof(Vertex, color));
    vertex_state.AddAtributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT,
                                        offsetof(Vertex, tex_coord));

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

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

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
    pipeline_info.layout = pipeline_.layout;
    pipeline_info.renderPass = render_pass_;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.subpass = 0;

    info.context->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipeline_.pipeline);
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

    Texture color_tex = alloc->CreateTexture(
        resource_->main_pass_resource_.color_image, color_view);
    resource_->main_pass_resource_.sampler_tex = color_tex;
    Texture depth_tex = alloc->CreateTexture(depth_image, depth_view);

    std::vector<VkImageView> attachments;
    attachments.push_back(color_tex.descriptor.imageView);
    attachments.push_back(depth_tex.descriptor.imageView);

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
void MainCameraPass::LoadTexture() {
    int tex_w = 0;
    int tex_h = 0;
    int tex_ch = 0;
    std::string image_file =
        "D:/project/cpp/graphics/vk-demo/image/texture.jpg";

    unsigned char* pixels = FileUtil::ReadImg(image_file, tex_w, tex_h, tex_ch);

    VkDeviceSize image_size = static_cast<long long>(tex_w * tex_h * 4);
    if (tex_w < 0 || tex_h < 0) {
        LogFatal("Read texture image error");
    }
    VkExtent2D extent = {static_cast<uint32_t>(tex_w),
                         static_cast<uint32_t>(tex_h)};
    VkImageCreateInfo create_info = MakeImage2DCreateInfo(extent);
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    RHIImage img = context_->GetAllocator()->CreateImage(
        cmd, image_size, pixels, create_info,
        /* VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL*/ VK_IMAGE_LAYOUT_GENERAL);

    VkImageViewCreateInfo view_create_info = MakeImage2DViewCreateInfo(
        img.image, VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM);
    VkSamplerCreateInfo sampler_create_info{};
    image_tex_ = context_->GetAllocator()->CreateTexture(img, view_create_info,
                                                         sampler_create_info);

    context_->GetCommandPool()->SubmitAndWait(cmd);
}
void MainCameraPass::UpdateUniform() {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     current_time - start_time)
                     .count();
    UniformBuffer ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0F), time * glm::radians(90.0F),
                            glm::vec3(0.0F, 0.0F, 1.0F));
    ubo.view =
        glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F),
                    glm::vec3(0.0F, 0.0F, 1.0F));

    ubo.proj =
        glm::perspective(glm::radians(45.0F), 1920 / (float)1080, 0.1F, 10.0F);
    ubo.proj[1][1] *= -1;

    void* data = context_->GetAllocator()->Map(kShaderBuffer.uniform);

    memcpy(data, &ubo, sizeof(ubo));
    context_->GetAllocator()->UnMap(kShaderBuffer.uniform);
}
// SaveImage for test and debug
void MainCameraPass::SaveImage() {
    VkRect2D* rect = context_->GetSwapchain()->GetScissor();
    VkDeviceSize mem_size = rect->extent.width * rect->extent.height * 4;
    Buffer buf = context_->GetAllocator()->CreateBuffer(
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