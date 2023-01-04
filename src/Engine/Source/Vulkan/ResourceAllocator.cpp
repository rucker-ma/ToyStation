#include "ResourceAllocator.h"

#include "VkImageUtil.h"

namespace toystation {

VkResourceAllocator::VkResourceAllocator(VkDevice device,
                                         VkPhysicalDevice physical_device,
                                         VkMemoryAllocator* mem_alloc,
                                         VkDeviceSize staging_block_size) {
    Init(device, physical_device, mem_alloc, staging_block_size);
}

VkResourceAllocator::~VkResourceAllocator() { DeInit(); }

void VkResourceAllocator::Init(VkDevice device,
                               VkPhysicalDevice physical_device,
                               VkMemoryAllocator* mem_alloc,
                               VkDeviceSize staging_block_size) {
    device_ = device;
    physical_device_ = physical_device;
    mem_alloc_ = mem_alloc;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties_);
    staging_ =
        std::make_unique<StagingMemoryManager>(mem_alloc, staging_block_size);
}

void VkResourceAllocator::DeInit() { staging_.reset(); }

Buffer VkResourceAllocator::CreateBuffer(const VkBufferCreateInfo& info,
                                         VkMemoryPropertyFlags mem_usage) {
    Buffer result_buffer;
    CreateBufferEx(info, &result_buffer.buffer);
    VkMemoryRequirements2 mem_reqs;
    ZeroVKStruct(mem_reqs, VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2);
    VkMemoryDedicatedRequirements dedicated_reqs;
    ZeroVKStruct(dedicated_reqs,
                 VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS);
    VkBufferMemoryRequirementsInfo2 buffer_reqs;
    ZeroVKStruct(buffer_reqs,
                 VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2);

    mem_reqs.pNext = &dedicated_reqs;
    buffer_reqs.buffer = result_buffer.buffer;
    vkGetBufferMemoryRequirements2(device_, &buffer_reqs, &mem_reqs);
    MemoryAllocateInfo alloc_info(mem_reqs.memoryRequirements, mem_usage,
                                  false);
    if (info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        alloc_info.SetAllocateFlags(VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
    }
    if (dedicated_reqs.requiresDedicatedAllocation) {
        alloc_info.SetBuffer(result_buffer.buffer);
    }
    result_buffer.handle = mem_alloc_->AllocMemory(alloc_info);
    if (result_buffer.handle) {
        const auto mem_info = mem_alloc_->GetMemoryInfo(result_buffer.handle);
        vkBindBufferMemory(device_, result_buffer.buffer, mem_info.memory,
                           mem_info.offset);
    } else {
        Destroy(result_buffer);
    }
    return result_buffer;
}

Buffer VkResourceAllocator::CreateBuffer(VkDeviceSize size,
                                         VkBufferUsageFlags usage,
                                         VkMemoryPropertyFlags mem_usage) {
    VkBufferCreateInfo info;
    ZeroVKStruct(info, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    info.size = size;
    info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    return CreateBuffer(info, mem_usage);
}
Buffer VkResourceAllocator::CreateBuffer(const VkCommandBuffer& cmd_buf,
                                         const VkDeviceSize& size,
                                         const void* data,
                                         VkBufferUsageFlags usage,
                                         VkMemoryPropertyFlags mem_props) {
    VkBufferCreateInfo info;
    ZeroVKStruct(info, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    info.size = size;
    info.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    Buffer result_buffer = CreateBuffer(info, mem_props);
    if (data) {
        // staging command to buffer
        staging_->CmdToBuffer(cmd_buf, result_buffer.buffer, 0, size, data);
    }
    return result_buffer;
}
RHIImage VkResourceAllocator::CreateImage(const VkImageCreateInfo& info,
                                          VkMemoryPropertyFlags mem_usage) {
    RHIImage image_result;
    CreateImageEx(info, &image_result.image);

    VkMemoryRequirements2 mem_reqs;
    ZeroVKStruct(mem_reqs, VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2);
    VkMemoryDedicatedRequirements dedicated_reqs;
    ZeroVKStruct(dedicated_reqs,
                 VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS);
    VkImageMemoryRequirementsInfo2 image_reqs;
    ZeroVKStruct(image_reqs,
                 VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2);
    image_reqs.image = image_result.image;
    mem_reqs.pNext = &dedicated_reqs;
    vkGetImageMemoryRequirements2(device_, &image_reqs, &mem_reqs);
    MemoryAllocateInfo alloc_info(mem_reqs.memoryRequirements, mem_usage, true);
    if (dedicated_reqs.prefersDedicatedAllocation) {
        alloc_info.SetImage(image_result.image);
    }
    image_result.handle = mem_alloc_->AllocMemory(alloc_info);
    if (image_result.handle) {
        const auto mem_info = mem_alloc_->GetMemoryInfo(image_result.handle);
        vkBindImageMemory(device_, image_result.image, mem_info.memory,
                          mem_info.offset);

    } else {
        Destroy(image_result);
    }

    return image_result;
}
RHIImage VkResourceAllocator::CreateImage(const VkCommandBuffer& cmd_buffer,
                                          size_t size, const void* data,
                                          const VkImageCreateInfo& info,
                                          VkImageLayout layout) {
    RHIImage result_image =
        CreateImage(info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (data != nullptr) {
        VkImageSubresourceRange subresource_range;
        subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource_range.baseArrayLayer = 0;
        subresource_range.baseMipLevel = 0;
        subresource_range.layerCount = 1;
        subresource_range.levelCount = info.mipLevels;
        VkImageUtil::CmdBarrierImageLayout(
            cmd_buffer, result_image.image, VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresource_range);
        VkOffset3D offset = {0};
        VkImageSubresourceLayers subresource = {0};
        subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresource.layerCount = 1;

        staging_->CmdToImage(cmd_buffer, result_image.image, offset,
                             info.extent, subresource, size, data);

        VkImageUtil::CmdBarrierImageLayout(cmd_buffer, result_image.image,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           layout, subresource_range);
    } else {
        // VkImageUtil::CmdBarrierImageLayout(cmd_buffer, result_image.image,
        //                            VK_IMAGE_LAYOUT_UNDEFINED,
        //                            layout);
    }
    return result_image;
}
Texture VkResourceAllocator::CreateTexture(
    const RHIImage& image, const VkImageViewCreateInfo& imageview_create_info) {
    Texture result_texture;
    result_texture.image = image.image;
    result_texture.handle = image.handle;
    result_texture.descriptor.imageLayout =
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    assert(imageview_create_info.image == image.image);
    vkCreateImageView(device_, &imageview_create_info, nullptr,
                      &result_texture.descriptor.imageView);

    return result_texture;
}
Texture VkResourceAllocator::CreateTexture(
    const RHIImage& image, const VkImageViewCreateInfo& imageview_create_info,
    const VkSamplerCreateInfo& sampler_create_info) {
    Texture result_texture = CreateTexture(image, imageview_create_info);
    // result_texture.descriptor.sampler =
    // m_samplerPool.acquireSampler(samplerCreateInfo);
    
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_FALSE;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device_, &properties);
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0;
    sampler_info.minLod = 0;
    sampler_info.maxLod = 0;

    vkCreateSampler(device_, &sampler_info, nullptr, &result_texture.descriptor.sampler);
    return result_texture;
}
Texture VkResourceAllocator::CreateTexture(
    const VkCommandBuffer& cmd_buffer, size_t size, const void* data,
    const VkImageCreateInfo& info, VkSamplerCreateInfo& sampler_create_info,
    VkImageLayout layout, bool is_cube) {
    RHIImage image = CreateImage(cmd_buffer, size, data, info, layout);

    VkImageViewCreateInfo view_info;
    ZeroVKStruct(view_info, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);

    view_info.pNext = nullptr;
    view_info.image = image.image;
    view_info.format = info.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
    switch (info.imageType) {
        case VK_IMAGE_TYPE_1D:
            view_info.viewType =
                (info.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY
                                      : VK_IMAGE_VIEW_TYPE_1D);
            break;
        case VK_IMAGE_TYPE_2D:
            view_info.viewType =
                is_cube ? VK_IMAGE_VIEW_TYPE_CUBE
                        : (info.arrayLayers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
                                                : VK_IMAGE_VIEW_TYPE_2D);
            break;
        case VK_IMAGE_TYPE_3D:
            view_info.viewType = VK_IMAGE_VIEW_TYPE_3D;
            break;
        default:
            assert(0);
    }

    Texture result_texture =
        CreateTexture(image, view_info, sampler_create_info);
    result_texture.descriptor.imageLayout = layout;
    return result_texture;
}
void VkResourceAllocator::Destroy(Buffer& buffer) {
    vkDestroyBuffer(device_, buffer.buffer, nullptr);
    mem_alloc_->FreeMemory(buffer.handle);
}
void VkResourceAllocator::Destroy(RHIImage& image) {
    vkDestroyImage(device_, image.image, nullptr);
    mem_alloc_->FreeMemory(image.handle);
}
void VkResourceAllocator::Destroy(Texture& texture) {
    vkDestroyImageView(device_, texture.descriptor.imageView, nullptr);
    vkDestroyImage(device_, texture.image, nullptr);
    mem_alloc_->FreeMemory(texture.handle);

    if (texture.descriptor.sampler) {
        // TODO:添加完成sampler部分
        // m_samplerPool.releaseSampler(t_.descriptor.sampler);
    }
}
void* VkResourceAllocator::Map(const Buffer& buffer) {
    void* data = mem_alloc_->Map(buffer.handle);
    return data;
}
void VkResourceAllocator::UnMap(const Buffer& buffer) {
    mem_alloc_->UnMap(buffer.handle);
}
void* VkResourceAllocator::Map(const RHIImage& image) {
    void* data = mem_alloc_->Map(image.handle);
    return data;
}
void VkResourceAllocator::UnMap(const RHIImage& image) {
    mem_alloc_->UnMap(image.handle);
}
void VkResourceAllocator::CreateBufferEx(const VkBufferCreateInfo& info,
                                         VkBuffer* buffer) {
    vkCreateBuffer(device_, &info, nullptr, buffer);
}
void VkResourceAllocator::CreateImageEx(const VkImageCreateInfo& info,
                                        VkImage* image) {
    vkCreateImage(device_, &info, nullptr, image);
}

//---------------------DedicatedResourceAllocator--------------------//

DedicatedResourceAllocator::DedicatedResourceAllocator(
    VkDevice device, VkPhysicalDevice physical_device,
    VkDeviceSize staging_block_size) {
    Init(device, physical_device, staging_block_size);
}
DedicatedResourceAllocator::~DedicatedResourceAllocator() { DeInit(); }
void DedicatedResourceAllocator::Init(VkContext* ctx,
                                      VkDeviceSize staging_block_size) {
    Init(ctx->GetDevice(), ctx->GetPhysicalDevice(), staging_block_size);
}
void DedicatedResourceAllocator::Init(VkDevice device,
                                      VkPhysicalDevice physical_device,
                                      VkDeviceSize staging_block_size) {
    mem_alloc_ = std::make_unique<VkMemoryAllocator>();
    mem_alloc_->Init(device, physical_device);
    VkResourceAllocator::Init(device, physical_device, mem_alloc_.get(),
                              staging_block_size);
}
void DedicatedResourceAllocator::Init(VkInstance instance, VkDevice device,
                                      VkPhysicalDevice physical_device,
                                      VkDeviceSize staging_block_size) {
    Init(device, physical_device, staging_block_size);
}
void DedicatedResourceAllocator::DeInit() {}
}  // namespace toystation