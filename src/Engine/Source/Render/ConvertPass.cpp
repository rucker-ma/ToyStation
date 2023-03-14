#include "ConvertPass.h"

#include "MainCameraPass.h"
#include "RenderFrame.h"
#include "Vulkan/Images.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/VkImageUtil.h"

namespace toystation {
void FrameConvertYCrCbPass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    render_pass_ = resource_->current_pass_;
    SetupTexture(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
}
void FrameConvertYCrCbPass::Draw() {
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_.pipeline);

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageUtil::CmdBarrierImageLayout(
        cmd, resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL, sub_range);

    vkCmdBindDescriptorSets(
        cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_.layout, 0, 1,
        &convert_resource_.set_container.GetSet(0), 0, nullptr);

    VkExtent2D extent = context_->GetSwapchain()->GetScissor()->extent;

    vkCmdDispatch(cmd, extent.width / 16, extent.height / 8, 1);

    VkImageUtil::CmdBarrierImageLayout(
        cmd, resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, sub_range);

    context_->GetCommandPool()->SubmitAndWait(cmd);

    // RenderContext::SaveToImage("666.bmp",convert_resource_.yuv_image,
    // VK_IMAGE_LAYOUT_GENERAL,VK_FORMAT_R8G8B8A8_UNORM,context_);
}

void FrameConvertYCrCbPass::SetupDescriptorSetLayout(RenderPassInitInfo& info) {
    convert_resource_.set_container.Init(info.context->GetContext().get());
    convert_resource_.set_container.AddBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    convert_resource_.set_container.AddBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    convert_resource_.set_container.AddBinding(
        2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    convert_resource_.set_container.AddBinding(
        3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    descriptor_.layout = convert_resource_.set_container.InitLayout();
    convert_resource_.set_container.InitPool(4);

    std::vector<VkWriteDescriptorSet> sets;
    // index 0: render渲染的rgba图像
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 0, &info.resource->main_pass_resource_.sampler_tex.descriptor));
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 1, &convert_resource_.tex_y.descriptor));
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 2, &convert_resource_.tex_cb.descriptor));
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 3, &convert_resource_.tex_cr.descriptor));
    convert_resource_.set_container.UpdateSets(sets);
}

void FrameConvertYCrCbPass::SetupPipeline(RenderPassInitInfo& info) {
    VkPipelineShaderStageCreateInfo compute_stage_info{};
    ZeroVKStruct(compute_stage_info,
                 VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    compute_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compute_stage_info.module =
        GetShader("D:/project/ToyStation/src/Engine/Shader/comp.spv",
                  info.context->GetContext());
    compute_stage_info.pName = "main";

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_.layout;
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipeline_.layout);

    VkComputePipelineCreateInfo pipeline_info;
    ZeroVKStruct(pipeline_info, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);

    pipeline_info.stage = compute_stage_info;
    pipeline_info.layout = pipeline_.layout;

    info.context->GetContext()->CreateComputePipeline(1, &pipeline_info,
                                                      pipeline_.pipeline);
}

void FrameConvertYCrCbPass::SetupTexture(RenderPassInitInfo& info) {
    auto alloc = info.context->GetAllocator();
    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();
    VkImageCreateInfo y_create_info = MakeImage2DCreateInfo(
        scissor->extent, VK_FORMAT_R8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    convert_resource_.comp_y = alloc->CreateImage(y_create_info);

    VkExtent2D uv_extent =
        VkExtent2D{scissor->extent.width / 2, scissor->extent.height / 2};
    VkImageCreateInfo uv_create_info = MakeImage2DCreateInfo(
        uv_extent, VK_FORMAT_R8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

    convert_resource_.comp_cb = alloc->CreateImage(uv_create_info);
    convert_resource_.comp_cr = alloc->CreateImage(uv_create_info);

    // component y
    VkImageViewCreateInfo y_view_create_info = MakeImage2DViewCreateInfo(
        convert_resource_.comp_y.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8_UNORM);

    convert_resource_.tex_y =
        alloc->CreateTexture(convert_resource_.comp_y, y_view_create_info);

    // component cb
    VkImageViewCreateInfo cb_view_create_info = MakeImage2DViewCreateInfo(
        convert_resource_.comp_cb.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8_UNORM);

    convert_resource_.tex_cb =
        alloc->CreateTexture(convert_resource_.comp_cb, cb_view_create_info);

    // component cr
    VkImageViewCreateInfo cr_view_create_info = MakeImage2DViewCreateInfo(
        convert_resource_.comp_cr.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8_UNORM);

    convert_resource_.tex_cr =
        alloc->CreateTexture(convert_resource_.comp_cr, cr_view_create_info);

    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageUtil::CmdBarrierImageLayout(cmd, convert_resource_.comp_y.image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_GENERAL, sub_range);

    VkImageUtil::CmdBarrierImageLayout(cmd, convert_resource_.comp_cb.image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_GENERAL, sub_range);

    VkImageUtil::CmdBarrierImageLayout(cmd, convert_resource_.comp_cr.image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_GENERAL, sub_range);
    context_->GetCommandPool()->SubmitAndWait(cmd);
}
std::unique_ptr<RenderFrame> FrameConvertYCrCbPass::GetConvertFrame() {
    return std::make_unique<RenderFrameYCbCr>(
        context_, convert_resource_.comp_y, convert_resource_.comp_cb,
        convert_resource_.comp_cr);
}
// ------------------------FrameConvertNV12Pass--------------------------------//
void FrameConvertNV12Pass::Initialize(RenderPassInitInfo& info) {
    context_ = info.context;
    resource_ = info.resource;
    render_pass_ = resource_->current_pass_;
    SetupTexture(info);
    SetupDescriptorSetLayout(info);
    SetupPipeline(info);
}
void FrameConvertNV12Pass::Draw() {
    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_.pipeline);

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkImageUtil::CmdBarrierImageLayout(
        cmd, resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_GENERAL, sub_range);

    vkCmdBindDescriptorSets(
        cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_.layout, 0, 1,
        &convert_resource_.set_container.GetSet(0), 0, nullptr);

    VkExtent2D extent = context_->GetSwapchain()->GetScissor()->extent;

    vkCmdDispatch(cmd, extent.width / 16, extent.height / 8, 1);

    VkImageUtil::CmdBarrierImageLayout(
        cmd, resource_->main_pass_resource_.color_image.image,
        VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, sub_range);

    context_->GetCommandPool()->SubmitAndWait(cmd);
    // RenderContext::SaveToImage("666.bmp",convert_resource_.yuv_image,
    // VK_IMAGE_LAYOUT_GENERAL,VK_FORMAT_R8G8B8A8_UNORM,context_);
}

void FrameConvertNV12Pass::SetupDescriptorSetLayout(RenderPassInitInfo& info) {
    convert_resource_.set_container.Init(info.context->GetContext().get());
    convert_resource_.set_container.AddBinding(
        0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    convert_resource_.set_container.AddBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    convert_resource_.set_container.AddBinding(
        2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT);

    descriptor_.layout = convert_resource_.set_container.InitLayout();
    convert_resource_.set_container.InitPool(3);

    std::vector<VkWriteDescriptorSet> sets;
    // index 0: render渲染的rgba图像
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 0, &info.resource->main_pass_resource_.sampler_tex.descriptor));
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 1, &convert_resource_.tex_y.descriptor));
    sets.push_back(convert_resource_.set_container.MakeWrite(
        0, 2, &convert_resource_.tex_uv.descriptor));

    convert_resource_.set_container.UpdateSets(sets);
}

void FrameConvertNV12Pass::SetupPipeline(RenderPassInitInfo& info) {
    VkPipelineShaderStageCreateInfo compute_stage_info{};
    ZeroVKStruct(compute_stage_info,
                 VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    compute_stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    compute_stage_info.module =
        GetShader("D:/project/ToyStation/src/Engine/Shader/convert_nv12.spv",
                  info.context->GetContext());
    compute_stage_info.pName = "main";

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_.layout;
    info.context->GetContext()->CreatePipelineLayout(pipeline_layout_info,
                                                     pipeline_.layout);

    VkComputePipelineCreateInfo pipeline_info;
    ZeroVKStruct(pipeline_info, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);

    pipeline_info.stage = compute_stage_info;
    pipeline_info.layout = pipeline_.layout;

    info.context->GetContext()->CreateComputePipeline(1, &pipeline_info,
                                                      pipeline_.pipeline);
}

void FrameConvertNV12Pass::SetupTexture(RenderPassInitInfo& info) {
    auto alloc = info.context->GetAllocator();
    VkRect2D* scissor = context_->GetSwapchain()->GetScissor();
    VkImageCreateInfo y_create_info = MakeImage2DCreateInfo(
        scissor->extent, VK_FORMAT_R8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    convert_resource_.comp_y = alloc->CreateImage(y_create_info);

    VkExtent2D uv_extent =
        VkExtent2D{scissor->extent.width, scissor->extent.height / 2};
    VkImageCreateInfo uv_create_info = MakeImage2DCreateInfo(
        uv_extent, VK_FORMAT_R8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);

    convert_resource_.comp_uv = alloc->CreateImage(uv_create_info);
    // component y
    VkImageViewCreateInfo y_view_create_info = MakeImage2DViewCreateInfo(
        convert_resource_.comp_y.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8_UNORM);

    convert_resource_.tex_y =
        alloc->CreateTexture(convert_resource_.comp_y, y_view_create_info);

    // component uv
    VkImageViewCreateInfo cb_view_create_info = MakeImage2DViewCreateInfo(
        convert_resource_.comp_uv.image, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8_UNORM);

    convert_resource_.tex_uv =
        alloc->CreateTexture(convert_resource_.comp_uv, cb_view_create_info);

    VkCommandBuffer cmd = context_->GetCommandPool()->CreateCommandBuffer(
        VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkImageSubresourceRange sub_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    VkImageUtil::CmdBarrierImageLayout(cmd, convert_resource_.comp_y.image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_GENERAL, sub_range);

    VkImageUtil::CmdBarrierImageLayout(cmd, convert_resource_.comp_uv.image,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_GENERAL, sub_range);

    context_->GetCommandPool()->SubmitAndWait(cmd);
}
std::unique_ptr<RenderFrame> FrameConvertNV12Pass::GetConvertFrame() {
    return std::make_unique<RenderFrameNV12>(context_, convert_resource_.comp_y,
                                             convert_resource_.comp_uv);
}
}  // namespace toystation