#pragma once

#include <memory>

#include "Image.h"
#include "RenderUtils.h"
namespace toystation {
class ImageFactory {
public:
    static std::shared_ptr<Image> CreateImage(std::string ImgFile,
                                              RenderUtils* Utils);
    static std::shared_ptr<Image> CreateImage(VkImage Img);
    static std::shared_ptr<Image> CreateImage(uint32_t width, uint32_t height,
                                              VkFormat format,
                                              VkImageTiling tiling,
                                              VkImageUsageFlags usage,
                                              VkMemoryPropertyFlags properties);
    static void BindMemory(Image& Image, VkMemoryPropertyFlags properties);
    static void SwapImageBindMemory(Image& Image,
                                    VkMemoryPropertyFlags properties,
                                    VkSwapchainKHR chain, uint32_t idx);

private:
    static void CreateVKImage(Image& image, int Width, int Height,
                              VkFormat Format, VkImageTiling tiling,
                              VkImageUsageFlags usage,
                              VkMemoryPropertyFlags properties);
};
}  // namespace toystation
