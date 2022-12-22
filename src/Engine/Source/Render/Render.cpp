#include "Render.h"

#include <array>
#include <chrono>
#include <stdexcept>
#include <vector>

#include "Shader.h"

namespace TSEngine {
Render::~Render() { CleanSwapChain(); }
void Render::Init() {
    info_.SwapChain = nullptr;
    CreateSwapChain();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateUtils();
    CreateTextures();
    CreateFrameBuffers();
    CreateUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffer();
    CreateSyncObjects();
    CreatePresentImage();
    current_frame_ = 0;
    scale_ = 1;
    frame_resized_ = false;
}
void Render::RecreateSwapChain() {
    CleanSwapChain();

    CreateSwapChain();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateTextures();
    CreateFrameBuffers();
    CreateUniformBuffer();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffer();
    CreatePresentImage();
    current_frame_ = 0;
}
void Render::CleanSwapChain() {
    vkDeviceWaitIdle(VulkanDevice);
    depth_texture_ = nullptr;
    for (auto& frame : swapchain_framebuffers_) {
        vkDestroyFramebuffer(VulkanDevice, frame, nullptr);
    }
    vkDestroyDescriptorPool(VulkanDevice, descriptor_pool_, nullptr);
    vkDestroyPipeline(VulkanDevice, graphics_pipeline_, nullptr);
    vkDestroyPipelineLayout(VulkanDevice, pipeline_layout_, nullptr);
    vkDestroyRenderPass(VulkanDevice, render_pass_, nullptr);

    swap_chain_images_.clear();

    // vkDestroySwapchainKHR(VulkanDevice, info_.SwapChain, nullptr);
    for (auto& buffer : uniform_buffers_) {
        delete buffer;
        buffer = nullptr;
    }
    uniform_buffers_.clear();
}
void Render::Draw() {
    vkWaitForFences(VulkanDevice, 1, &inflight_fences_[current_frame_], VK_TRUE,
                    UINT64_MAX);

    uint32_t image_index = 0;
    VkResult result =
        vkAcquireNextImageKHR(VulkanDevice, info_.SwapChain, UINT64_MAX,
                              image_available_semaphores_[current_frame_],
                              VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // todo: recreate swap chain when surface size changed
        RecreateSwapChain();
        return;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    }
    vkResetFences(VulkanDevice, 1, &inflight_fences_[current_frame_]);
    vkResetCommandBuffer(commandbuffers_[current_frame_], 0);
    RecordCommandBuffer(commandbuffers_[current_frame_], image_index);

    UpdateUniformBuffer(current_frame_);
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {
        image_available_semaphores_[current_frame_]};
    VkPipelineStageFlags wait_stages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &commandbuffers_[current_frame_];

    VkSemaphore signal_semaphore[] = {
        render_finished_semaphores_[current_frame_]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = static_cast<VkSemaphore*>(signal_semaphore);
    if (vkQueueSubmit(VulkanGraphicsQueue, 1, &submit_info,
                      inflight_fences_[current_frame_]) != VK_SUCCESS) {
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphore;
    VkSwapchainKHR swap_chains[] = {info_.SwapChain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(VulkanPresentQueue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        frame_resized_) {
        frame_resized_ = false;
        RecreateSwapChain();
    } else if (result != VK_SUCCESS) {
    }
    current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}
ImageInfo Render::GetNextImage(VkExtent2D Size) {
    if ((Size.width != info_.Extend.width) ||
        (Size.height != info_.Extend.height)) {
        RecreateSwapChain();
    }
    ImageInfo present_image_info_;

    present_image_info_.image =
        reinterpret_cast<std::uintptr_t>(present_image->GetVkImage());
    present_image_info_.image_view = 0;
    present_image_info_.memory =
        reinterpret_cast<std::uintptr_t>(present_image->GetMemory());
    present_image_info_.width = info_.Extend.width;
    present_image_info_.height = info_.Extend.height;
    present_image_info_.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    present_image_info_.tiling = VK_IMAGE_TILING_OPTIMAL;
    present_image_info_.format = VK_FORMAT_B8G8R8A8_UNORM;
    present_image_info_.usage_flags =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    present_image_info_.memory_size = present_image->GetMemorySize();
    return present_image_info_;
}

void Render::UpdateSize(VkRect2D Size) { copy_size_ = Size; }

void Render::SetScale(double Scale) { scale_ = Scale; }

void Render::FrameResized() { frame_resized_ = true; }

void Render::CreateSwapChain() {
    VulkanSwapChainSupportDetails swapChainSupport =
        VulkanContext::Instance().QuerySwapChainSupport(VulkanPhysicsDevice);

    VkSurfaceFormatKHR surfaceFormat = swapChainSupport.formats[0];
    for (const auto& available_format : swapChainSupport.formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = available_format;
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // for (const auto& availablePresentMode : swapChainSupport.present_modes)
    //{
    //     if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
    //     {
    //         presentMode = availablePresentMode;
    //     }
    // }
    VkExtent2D extent = swapChainSupport.capabilities.currentExtent;

    // VkExtent2D extent =
    // vulkan_choose_swap_extent(swapChainSupport.capabilities,
    // init_params.window);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = VulkanSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    VulkanQueueFamily indices =
        VulkanContext::Instance().FindQueueFamilies(VulkanPhysicsDevice);
    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(),
                                     indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = present_mode;
    createInfo.clipped = VK_TRUE;
    if (info_.SwapChain == nullptr) {
        createInfo.oldSwapchain = VK_NULL_HANDLE;
    } else {
        createInfo.oldSwapchain = info_.SwapChain;
    }

    // auto res =
    vkCreateSwapchainKHR(VulkanDevice, &createInfo, nullptr, &info_.SwapChain);

    vkGetSwapchainImagesKHR(VulkanDevice, info_.SwapChain, &imageCount,
                            nullptr);

    std::vector<VkImage> images(imageCount);

    vkGetSwapchainImagesKHR(VulkanDevice, info_.SwapChain, &imageCount,
                            images.data());

    info_.SurfaceFormat = surfaceFormat.format;
    info_.Extend = extent;

    swap_chain_images_.resize(imageCount);
    auto iter = swap_chain_images_.begin();
    for (auto img : images) {
        *iter = ImageFactory::CreateImage(img);
        (*iter)->GetVkImageView(info_.SurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        iter++;
    }
}
void Render::CreateCommandPool() {
    VulkanQueueFamily queue_indices =
        VulkanContext::Instance().FindQueueFamilies(VulkanPhysicsDevice);
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_indices.graphics_family.value();
    if (vkCreateCommandPool(VulkanDevice, &pool_info, nullptr,
                            &command_pool_) != VK_SUCCESS) {
    }
}
void Render::CreateRenderPass() {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = info_.SurfaceFormat;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment{};
    depth_attachment.format = FindDepthFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {color_attachment,
                                                          depth_attachment};
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = attachments.size();
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;
    if (vkCreateRenderPass(VulkanDevice, &render_pass_info, nullptr,
                           &render_pass_) != VK_SUCCESS) {
    }
}

void Render::CreateGraphicsPipeline() {
    std::string vertex_shader_file =
        "D:/project/cpp/graphics/vk-demo/shader/vert.spv";
    std::string frag_shader_file =
        "D:/project/cpp/graphics/vk-demo/shader/frag.spv";

    VkPipelineShaderStageCreateInfo vert_shaderstage_info{};
    vert_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shaderstage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shaderstage_info.module =
        Shader(vertex_shader_file).GetVKShaderModule();
    vert_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shaderstage_info{};
    frag_shaderstage_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shaderstage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shaderstage_info.module = Shader(frag_shader_file).GetVKShaderModule();
    ;
    frag_shaderstage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shaderstage_info,
                                                       frag_shaderstage_info};

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    auto binding_desc = Vertex::GetBindingDescription();
    auto attribute_desc = Vertex::GetAttributeDescriptions();
    vertex_input_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_desc;
    vertex_input_info.vertexAttributeDescriptionCount = attribute_desc.size();
    vertex_input_info.pVertexAttributeDescriptions = attribute_desc.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkExtent2D Extent = info_.Extend;

    VkViewport view_port{};
    view_port.x = 0;
    view_port.y = 0;
    view_port.width = Extent.width;
    view_port.height = Extent.height;
    view_port.minDepth = 0;
    view_port.maxDepth = 1;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = Extent;

    VkPipelineViewportStateCreateInfo view_port_state{};
    view_port_state.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    view_port_state.viewportCount = 1;
    view_port_state.pViewports = &view_port;
    view_port_state.scissorCount = 1;
    view_port_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;

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

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout_;
    if (vkCreatePipelineLayout(VulkanContext::Instance().Device(),
                               &pipeline_layout_info, nullptr,
                               &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &view_port_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = pipeline_layout_;
    pipeline_info.renderPass = render_pass_;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.subpass = 0;

    if (vkCreateGraphicsPipelines(VulkanContext::Instance().Device(),
                                  VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                  &graphics_pipeline_) != VK_SUCCESS) {
    }
}

void Render::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding ubo_layout_binding{};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        ubo_layout_binding, sampler_layout_binding};

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = bindings.size();
    layout_info.pBindings = bindings.data();
    if (vkCreateDescriptorSetLayout(VulkanDevice, &layout_info, nullptr,
                                    &descriptor_set_layout_) != VK_SUCCESS) {
    }
}

void Render::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = pool_sizes.size();
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = MAX_FRAMES_IN_FLIGHT;
    if (vkCreateDescriptorPool(VulkanDevice, &pool_info, nullptr,
                               &descriptor_pool_) != VK_SUCCESS) {
    }
}

void Render::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                               descriptor_set_layout_);
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts = layouts.data();

    uniform_buffers_.resize(MAX_FRAMES_IN_FLIGHT);
    descritpor_sets_.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(VulkanDevice, &alloc_info,
                                 descritpor_sets_.data()) != VK_SUCCESS) {
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_Info{};
        uniform_buffers_[i] = new GPUBuffer<UniformBufferOjbect>(
            utils_, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        buffer_Info.buffer = uniform_buffers_[i]->GetBuffer();
        buffer_Info.offset = 0;
        buffer_Info.range = sizeof(UniformBufferOjbect);

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = texture_->GetVkImageView(
            VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        image_info.sampler = texture_->GetImageSampler();

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = descritpor_sets_[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_Info;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = descritpor_sets_[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(VulkanDevice, descriptor_writes.size(),
                               descriptor_writes.data(), 0, nullptr);
    }
}

void Render::CreateCommandBuffer() {
    commandbuffers_.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = commandbuffers_.size();
    if (vkAllocateCommandBuffers(VulkanDevice, &alloc_info,
                                 commandbuffers_.data()) != VK_SUCCESS) {
    }
}

void Render::CreateSyncObjects() {
    image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    render_finished_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inflight_fences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(VulkanDevice, &semaphore_info, nullptr,
                              &image_available_semaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(VulkanDevice, &semaphore_info, nullptr,
                              &render_finished_semaphores_[i]) != VK_SUCCESS ||
            vkCreateFence(VulkanDevice, &fence_info, nullptr,
                          &inflight_fences_[i]) != VK_SUCCESS) {
        }
    }
}

void Render::CreateFrameBuffers() {
    swapchain_framebuffers_.resize(swap_chain_images_.size());

    for (size_t i = 0; i < swapchain_framebuffers_.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swap_chain_images_[i]->GetVkImageView(info_.SurfaceFormat,
                                                  VK_IMAGE_ASPECT_COLOR_BIT),
            depth_texture_->GetVkImageView(FindDepthFormat(),
                                           VK_IMAGE_ASPECT_DEPTH_BIT)};

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass_;
        framebuffer_info.attachmentCount = attachments.size();
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = info_.Extend.width;
        framebuffer_info.height = info_.Extend.height;
        framebuffer_info.layers = 1;
        if (vkCreateFramebuffer(VulkanDevice, &framebuffer_info, nullptr,
                                &swapchain_framebuffers_[i]) != VK_SUCCESS) {
        }
    }
}

void Render::CreatePresentImage() {
    present_image = ImageFactory::CreateImage(
        info_.Extend.width, info_.Extend.height, VK_FORMAT_B8G8R8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    utils_->TransitionImageLayout(
        present_image->GetVkImage(), VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_ACCESS_NONE, VK_ACCESS_NONE_KHR);
}

void Render::CreateTextures() {
    texture_ = ImageFactory::CreateImage(
        "D:/project/cpp/graphics/vk-demo/image/texture.jpg", utils_);
    texture_->GetVkImageView(VK_FORMAT_R8G8B8A8_SRGB,
                             VK_IMAGE_ASPECT_COLOR_BIT);
    texture_->GetImageSampler();

    depth_texture_ = ImageFactory::CreateImage(
        info_.Extend.width, info_.Extend.height, FindDepthFormat(),
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Render::CreateUniformBuffer() {
    vertex_buffer_ = new GPUBuffer<Vertex>(
        utils_, vertices,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    indices_buffer_ = new GPUBuffer<uint16_t>(
        utils_, indices,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

void Render::CreateUtils() { utils_ = new RenderUtils(command_pool_); }

VkFormat Render::FindDepthFormat() {
    const std::vector<VkFormat> candidates{VK_FORMAT_D32_SFLOAT,
                                           VK_FORMAT_D32_SFLOAT_S8_UINT,
                                           VK_FORMAT_D24_UNORM_S8_UINT};
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features =
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(VulkanPhysicsDevice, format,
                                            &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}
void Render::RecordCommandBuffer(VkCommandBuffer CommandBuffer,
                                 uint32_t image_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(CommandBuffer, &begin_info) != VK_SUCCESS) {
    }

    utils_->TransitionImageLayoutWithCommandBuffer(
        CommandBuffer, swap_chain_images_[current_frame_]->GetVkImage(),
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_ACCESS_NONE_KHR, VK_ACCESS_TRANSFER_WRITE_BIT);
    utils_->TransitionImageLayoutWithCommandBuffer(
        CommandBuffer, present_image->GetVkImage(), VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_NONE_KHR,
        VK_ACCESS_TRANSFER_READ_BIT);

    VkImageBlit blt_info{};
    VkOffset3D Offset1 = {0, 0, 0};
    VkOffset3D Offset2 = {(int32_t)info_.Extend.width,
                          (int32_t)info_.Extend.height, 1};
    blt_info.srcOffsets[0] = Offset1;
    blt_info.srcOffsets[1] = Offset2;

    blt_info.dstOffsets[0] = Offset1;
    blt_info.dstOffsets[1] = Offset2;
    VkImageSubresourceLayers Subresource =
        VkImageSubresourceLayers{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    blt_info.srcSubresource = Subresource;
    blt_info.dstSubresource = Subresource;
    vkCmdBlitImage(CommandBuffer, present_image->GetVkImage(),
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   swap_chain_images_[current_frame_]->GetVkImage(),
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blt_info,
                   VK_FILTER_LINEAR);
    utils_->TransitionImageLayoutWithCommandBuffer(
        CommandBuffer, present_image->GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
        VK_ACCESS_NONE_KHR);
    utils_->TransitionImageLayoutWithCommandBuffer(
        CommandBuffer, swap_chain_images_[current_frame_]->GetVkImage(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_ACCESS_NONE_KHR, VK_ACCESS_NONE_KHR);

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass_;
    render_pass_info.framebuffer = swapchain_framebuffers_[image_index];

    // if (copy_size_.width == 0 && copy_size_.height == 0) {
    //     render_pass_info.renderArea.offset = {0, 0};
    //     render_pass_info.renderArea.extent = info_.Extend;
    // } else {
    auto offset_x = static_cast<int32_t>(copy_size_.offset.x * scale_);
    auto offset_y = static_cast<int32_t>(copy_size_.offset.y * scale_);
    render_pass_info.renderArea.offset = {offset_x, offset_y};
    // render_pass_info.renderArea.extent = {
    //     info_.Extend.width - offset_x, info_.Extend.height - offset_y * 2};
    if (info_.Extend.width <
        (copy_size_.extent.width + copy_size_.offset.x) * scale_) {
        copy_size_.extent.width = info_.Extend.width - offset_x;
    }
    if (info_.Extend.height <
        (copy_size_.extent.height + copy_size_.offset.y) * scale_) {
        copy_size_.extent.height = info_.Extend.height - offset_y;
    }

    render_pass_info.renderArea.extent = {
        static_cast<uint32_t>(copy_size_.extent.width * scale_),
        static_cast<uint32_t>(copy_size_.extent.height * scale_)};
    //}

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
    clear_values[1].depthStencil = {1.0F, 0};
    render_pass_info.clearValueCount = clear_values.size();
    render_pass_info.pClearValues = clear_values.data();
    vkCmdBeginRenderPass(CommandBuffer, &render_pass_info,
                         VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      graphics_pipeline_);

    VkBuffer vertex_buffers[] = {vertex_buffer_->GetBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(CommandBuffer, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(CommandBuffer, indices_buffer_->GetBuffer(), 0,
                         VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline_layout_, 0, 1,
                            &descritpor_sets_[current_frame_], 0, nullptr);
    vkCmdDrawIndexed(CommandBuffer, indices.size(), 1, 0, 0, 0);
    vkCmdEndRenderPass(CommandBuffer);

    if (vkEndCommandBuffer(CommandBuffer) != VK_SUCCESS) {
    }
}
void Render::UpdateUniformBuffer(uint32_t current_image) {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();
    UniformBufferOjbect ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view =
        glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.proj = glm::perspective(glm::radians(45.0f),
                                info_.Extend.width / (float)info_.Extend.height,
                                0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
    void* data = nullptr;
    vkMapMemory(VulkanDevice, uniform_buffers_[current_image]->GetMemory(), 0,
                sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(VulkanDevice, uniform_buffers_[current_image]->GetMemory());
}

}  // namespace TSEngine
