#include "ImageFactory.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stdexcept>

#include "Render.h"
#include "stb_image.h"

namespace toystation {

std::shared_ptr<Image> ImageFactory::CreateImage(std::string ImgFile,
                                                 RenderUtils* Utils) {
    std::shared_ptr<Image> res(new Image());
    int tex_w = 0;
    int tex_h = 0;
    int tex_ch = 0;
    stbi_uc* pixels =
        stbi_load(ImgFile.c_str(), &tex_w, &tex_h, &tex_ch, STBI_rgb_alpha);
    VkDeviceSize image_size = static_cast<VkDeviceSize>(tex_w * tex_h * 4);
    if (!pixels) {
        return res;
    }

    VkBuffer staging_buffer = nullptr;
    VkDeviceMemory staging_buffer_memory = nullptr;
    Utils->CreateGPUBuffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           staging_buffer, staging_buffer_memory);
    void* data = nullptr;
    vkMapMemory(VulkanDevice, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, image_size);
    vkUnmapMemory(VulkanDevice, staging_buffer_memory);
    stbi_image_free(pixels);

    CreateVKImage(*res, tex_w, tex_h, VK_FORMAT_R8G8B8A8_SRGB,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Utils->TransitionImageLayout(res->image_, VK_IMAGE_LAYOUT_UNDEFINED,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, 0);

    Utils->CopyBufferToImage(staging_buffer, res->image_, tex_w, tex_h);
    Utils->TransitionImageLayout(
        res->image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, 0);
    vkDestroyBuffer(VulkanDevice, staging_buffer, nullptr);
    vkFreeMemory(VulkanDevice, staging_buffer_memory, nullptr);
    return res;
}

std::shared_ptr<Image> ImageFactory::CreateImage(VkImage Img) {
    std::shared_ptr<Image> res(new Image(true));
    res->image_ = Img;
    return res;
}

std::shared_ptr<Image> ImageFactory::CreateImage(
    uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
    std::shared_ptr<Image> res(new Image());
    CreateVKImage(*res, width, height, format, tiling, usage, properties);
    return res;
}

void ImageFactory::BindMemory(Image& Image, VkMemoryPropertyFlags properties) {
    VkMemoryRequirements mem_requires;
    vkGetImageMemoryRequirements(VulkanDevice, Image.image_, &mem_requires);
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requires.size;
    alloc_info.memoryTypeIndex = VulkanContext::Instance().FindMemoryType(
        mem_requires.memoryTypeBits, properties);
    if (vkAllocateMemory(VulkanDevice, &alloc_info, nullptr,
                         &Image.image_memory_) != VK_SUCCESS) {
    }
    Image.memory_size_ = mem_requires.size;

    vkBindImageMemory(VulkanDevice, Image.image_, Image.image_memory_, 0);
}

void ImageFactory::SwapImageBindMemory(Image& Image,
                                       VkMemoryPropertyFlags properties,
                                       VkSwapchainKHR chain, uint32_t idx) {
    VkMemoryRequirements mem_requires;
    vkGetImageMemoryRequirements(VulkanDevice, Image.image_, &mem_requires);
    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requires.size;
    alloc_info.memoryTypeIndex = VulkanContext::Instance().FindMemoryType(
        mem_requires.memoryTypeBits, properties);
    if (vkAllocateMemory(VulkanDevice, &alloc_info, nullptr,
                         &Image.image_memory_) != VK_SUCCESS) {
        throw std::runtime_error("cannot allocate memory");
    }
    VkBindImageMemoryInfo bind_info{};
    bind_info.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bind_info.image = Image.image_;
    bind_info.memoryOffset = 0;
    bind_info.memory = Image.image_memory_;

    VkBindImageMemorySwapchainInfoKHR swapchain_khr{};
    swapchain_khr.sType =
        VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_SWAPCHAIN_INFO_KHR;
    swapchain_khr.swapchain = chain;
    swapchain_khr.imageIndex = idx;

    bind_info.pNext = &swapchain_khr;

    Image.memory_size_ = mem_requires.size;

    vkBindImageMemory2(VulkanDevice, 1, &bind_info);
}

void ImageFactory::CreateVKImage(Image& image, int Width, int Height,
                                 VkFormat Format, VkImageTiling tiling,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties) {
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = Width;
    image_info.extent.height = Height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = Format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    if (vkCreateImage(VulkanDevice, &image_info, nullptr, &image.image_) !=
        VK_SUCCESS) {
    }
    BindMemory(image, properties);
}
}  // namespace toystation
