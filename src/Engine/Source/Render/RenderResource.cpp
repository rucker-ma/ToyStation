#include "RenderResource.h"
#include "Vulkan/Images.h"

namespace toystation{

void RenderResource::Initialize(std::shared_ptr<RenderContext> context){
    gbuffers.resize(GBUFFER_COUNT);

    auto alloc = context->GetAllocator();
    VkRect2D* scissor = context->GetSwapchain()->GetScissor();

    VkImageCreateInfo image_create_info = MakeImage2DCreateInfo(
        scissor->extent, context->GetSwapchain()->GetFormat(),
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    VkImageCreateInfo depth_create_info =
        MakeImage2DCreateInfo(scissor->extent, VK_FORMAT_D32_SFLOAT,
                              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    RHIImage render_image = alloc->CreateImage(image_create_info);
    RHIImage depth_image = alloc->CreateImage(depth_create_info);

    VkImageViewCreateInfo color_view = MakeImage2DViewCreateInfo(
        render_image.image,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM);
    VkImageViewCreateInfo depth_view = MakeImage2DViewCreateInfo(
        depth_image.image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_FORMAT_D32_SFLOAT);

    shading_texture = alloc->CreateTexture(render_image, color_view);
    depth_texture = alloc->CreateTexture(depth_image, depth_view);
    //创建gbuffer
    VkImageCreateInfo gbuffer_create_info = MakeImage2DCreateInfo(
        scissor->extent,VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    for(auto& gbuffer:gbuffers){
        RHIImage gbuffer_image =  alloc->CreateImage(gbuffer_create_info);
        VkImageViewCreateInfo gbuffer_view = MakeImage2DViewCreateInfo(
            gbuffer_image.image,
            VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R32G32B32A32_SFLOAT);
        gbuffer = alloc->CreateTexture(gbuffer_image, gbuffer_view,true);
    }
    ubo_buffer_ = alloc->CreateBuffer(sizeof(toystation::UniformBuffer),
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    //ubo_.model = Matrix4(1.0);
}

}