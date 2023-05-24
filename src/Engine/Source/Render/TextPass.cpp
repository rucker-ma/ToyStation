#include "TextPass.h"

#include "Base/Logger.h"
#include "Compiler/ShaderCompilerSystem.h"
#include "File/FileUtil.h"
#include "Vulkan/Images.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VkImageUtil.h"

namespace toystation {
void TextPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    pipelines_.resize(SubPass_Count);
    if (FT_Init_FreeType(&ft_)) {
        LogError("Could not init FreeType Library");
        return;
    }
    std::string font_path = FileUtil::Combine("resource/font/arial.ttf");
    if(FT_New_Face(ft_,font_path.c_str(),0,&face_)){
        LogError("Failed to Load font");
        return;
    }
    FT_Set_Pixel_Sizes(face_,0,48);

    SetupRenderPass(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
    SetupFrameBuffer(info);

    PrepareRenderString({100,100},"ToyStation");
}

TextPass::~TextPass(){
    FT_Done_Face(face_);
    FT_Done_FreeType(ft_);
}
void TextPass::PostInitialize(){
    std::vector<VkWriteDescriptorSet> sets;

    VkDescriptorBufferInfo buffer_info;
    buffer_info.buffer = resource_->ubo_buffer_.buffer;
    buffer_info.offset = 0;
    buffer_info.range = sizeof(toystation::UniformBuffer);

    sets.push_back(set_container_.MakeWrite(
        0, 0, &buffer_info));
    sets.push_back(set_container_.MakeWrite(
        0, 1, &glyph_texture_.descriptor));
    set_container_.UpdateSets(sets);
}
void TextPass::Draw() {
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

    UpdateUniform();

    vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_GRAPHICS,pipelines_[SubPass_Default].layout,0,1,
                            &set_container_.GetSet(0),0,nullptr);
    VkDeviceSize  offset = {};
    vkCmdBindVertexBuffers(cmd,0,1,&vertex_buffer_.buffer,&offset);

    vkCmdDraw(cmd,current_render_char_.size(),1,0,0);
    vkCmdEndRenderPass(cmd);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}
void TextPass::PrepareRenderString(IVector2 position,std::string content){
    for (int i = 0; i < content.size(); ++i) {
        char c = content[i];
        if(chars_.find(c) == chars_.end()){
            LoadChar(c);
        }
        Character info = chars_[c];
        //左上角为（0，0）点
        Vector3  v_leftup = {(position.x+info.bearing.x)/(float)tex_size_.x,
                            (position.y-info.bearing.y)/(float)tex_size_.y,0};
        Vector3  v_rightup = {(position.x+info.bearing.x+info.size.x)/(float)tex_size_.x,
                            (position.y-info.bearing.y)/(float)tex_size_.y,0};
        Vector3  v_leftdown = {(position.x+info.bearing.x)/(float)tex_size_.x,
                             (position.y-info.bearing.y+info.size.y)/(float)tex_size_.y,0};
        Vector3  v_rightdown = {(position.x+info.bearing.x+info.size.x)/(float)tex_size_.x,
                              (position.y-info.bearing.y+info.size.y)/(float)tex_size_.y,0};
//        current_render_char_.push_back({v_leftup,info.coords.leftup});
//        current_render_char_.push_back({v_rightup,info.coords.rightup});
//        current_render_char_.push_back({v_leftdown,info.coords.leftdown});
//
//        current_render_char_.push_back({v_rightup,info.coords.rightup});
//        current_render_char_.push_back({v_leftdown,info.coords.leftdown});
//        current_render_char_.push_back({v_rightdown,info.coords.rightdown});

        current_render_char_.push_back({
            {position.x+info.bearing.x,position.y-info.bearing.y,0},info.coords.leftup
        });

        current_render_char_.push_back({
            {position.x+info.bearing.x+info.size.x,position.y-info.bearing.y,0},info.coords.rightup
        });

        current_render_char_.push_back({
            {position.x+info.bearing.x,position.y-info.bearing.y+info.size.y,0},info.coords.leftdown
        });

        current_render_char_.push_back({
            {position.x+info.bearing.x+info.size.x,position.y-info.bearing.y,0},info.coords.rightup
        });

        current_render_char_.push_back({
            {position.x+info.bearing.x,position.y-info.bearing.y+info.size.y,0},info.coords.leftdown
        });

        current_render_char_.push_back({
            {position.x+info.bearing.x+info.size.x,position.y-info.bearing.y+info.size.y,0},info.coords.rightdown
        });
        position.x = position.x+ info.advance.x/64.0f;
    }

    size_t buffer_size = sizeof(CharVertex)*current_render_char_.size();
    auto cmd = context_->GetCommandPool()->CreateCommandBuffer();
    vertex_buffer_ = context_->GetAllocator()->CreateBuffer(cmd,buffer_size,current_render_char_.data(),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}
void TextPass::LoadChar(unsigned long char_code){
    if(FT_Load_Char(face_,char_code,FT_LOAD_RENDER)){
        LogError("Failed to load Glyph");
        return;
    }
    IVector2 size{face_->glyph->bitmap.width,face_->glyph->bitmap.rows};
    IVector2 bearing{face_->glyph->bitmap_left,face_->glyph->bitmap_top};
    //advance.x单位为1/64像素
    IVector2 advance{face_->glyph->advance.x,face_->glyph->advance.y};
    Character char_info = {size,bearing,advance,CharCoord{}};

    int buffer_size = face_->glyph->bitmap.pitch*face_->glyph->bitmap.rows;
    auto cmd = context_->GetCommandPool()->CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    RHIBuffer rhi_buffer = context_->GetAllocator()->CreateBuffer(cmd,buffer_size,face_->glyph->bitmap.buffer,
                                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    //一行字符已经排满，移动到下一行从左开始继续排
    if(tex_origin_.x+advance.x/64.0f>tex_size_.x){
        tex_origin_.x=0;
        tex_origin_.y = next_y_;
        next_y_ = 0;
    }

    VkImageSubresourceLayers  subresource{};
    subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource.baseArrayLayer =0;
    subresource.mipLevel = 0;
    subresource.layerCount = 1;

    VkBufferImageCopy image_copy;
    image_copy.bufferOffset =0;
    image_copy.bufferRowLength = size.x;
    image_copy.bufferImageHeight = size.y;
    image_copy.imageSubresource = subresource;
    image_copy.imageOffset = {tex_origin_.x,tex_origin_.y,0};
    image_copy.imageExtent = {(uint32_t)size.x,(uint32_t)size.y,1};

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageUtil::CmdBarrierImageLayout(
        cmd,  glyph_texture_.image,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        sub_range);
    vkCmdCopyBufferToImage(cmd,rhi_buffer.buffer,glyph_texture_.image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,
                           &image_copy);
    VkImageUtil::CmdBarrierImageLayout(
        cmd,  glyph_texture_.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
        sub_range);
    context_->GetCommandPool()->SubmitAndWait(cmd);

    next_y_ = std::max(next_y_,size.y);
    AddChar(char_code,tex_origin_,char_info);
    tex_origin_.x +=std::ceil(advance.x/64.0f);

}
void TextPass::AddChar(unsigned long char_code,Vector2 position,Character info){

    info.coords.leftup = {position.x/tex_size_.x,position.y/tex_size_.y};
    info.coords.rightup = {(position.x+info.size.x)/tex_size_.x,position.y/tex_size_.y};
    info.coords.leftdown = {position.x/tex_size_.x,(position.y+info.size.y)/tex_size_.y};
    info.coords.rightdown = {(position.x+info.size.x)/tex_size_.x,
                             (position.y+info.size.y)/tex_size_.y};

    chars_[char_code] = info;
}
//设置文字为正交投影
void TextPass::UpdateUniform(){
    resource_->ubo_.proj = glm::ortho<float>(0,tex_size_.x,0,tex_size_.y,0.1F,100.0F);
    void*data = context_->GetAllocator()->Map(resource_->ubo_buffer_);
    memcpy(data,&resource_->ubo_,sizeof(resource_->ubo_));
    context_->GetAllocator()->UnMap(resource_->ubo_buffer_);
}
void TextPass::SetupRenderPass(RenderPassInitInfo& info){
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
void TextPass::SetupDescriptorSetLayout(RenderPassInitInfo& info){
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
void TextPass::ResetPass(){
    context_->GetContext()->DestroyPipeline(pipelines_[SubPass_Default].pipeline);
    RenderPassInitInfo info = {context_,resource_};
    SetupPipeline(info);
}
void TextPass::SetupPipeline(RenderPassInitInfo& info){

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount =  descriptor_.layout.size();
    pipeline_layout_info.pSetLayouts = descriptor_.layout.data();
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipelines_[SubPass_Default].layout);

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages =
        VkHelper::CreateShaderState(
            context_->GetContext(),
            ShaderCompilerSystem::kCompileResult.at(kTextPassVert),
            ShaderCompilerSystem::kCompileResult.at(kTextPassFrag));

    PipelineVertexState vertex_state;
    vertex_state.AddBindingDescription(0,sizeof(CharVertex),VK_VERTEX_INPUT_RATE_VERTEX);

    vertex_state.AddAtributeDescription(0,0,VK_FORMAT_R32G32B32_SFLOAT,0);
    vertex_state.AddAtributeDescription(0,1,VK_FORMAT_R32G32_SFLOAT,sizeof(Vector3));


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
    color_blend_attachment.blendEnable = VK_TRUE; //enable blend for text output to shading texture
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

    info.context->GetContext()->CreateGraphicsPipeline(1, &pipeline_info,
                                                       pipelines_[SubPass_Default].pipeline);
}
void TextPass::SetupFrameBuffer(RenderPassInitInfo& info){
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

    //VkRect2D* scissor = context_->GetSwapchain()->GetScissor();
    VkImageCreateInfo image_create_info = MakeImage2DCreateInfo(
        scissor->extent, VK_FORMAT_R8_UNORM,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    auto alloc = context_->GetAllocator();
    RHIImage glyph_image = alloc->CreateImage(image_create_info);
    VkImageViewCreateInfo color_view = MakeImage2DViewCreateInfo(
        glyph_image.image,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8_UNORM);
    tex_size_ = {scissor->extent.width,scissor->extent.height};
    VkSamplerCreateInfo sampler_create_info{};
    glyph_texture_ = alloc->CreateTexture(glyph_image, color_view,sampler_create_info);

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    auto cmd = context_->GetCommandPool()->CreateCommandBuffer();
    VkImageUtil::CmdBarrierImageLayout(
        cmd,  glyph_texture_.image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
        sub_range);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}

}  // namespace toystation