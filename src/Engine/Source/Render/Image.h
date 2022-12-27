#pragma once
#include <string>

#include "VulkanContext.h"

namespace toystation {
class Image {
    friend class ImageFactory;

public:
    ~Image();
    const VkImage& GetVkImage();
    const VkImageView& GetVkImageView(VkFormat Format,
                                      VkImageAspectFlags AspectFlags);
    void FreeImageView();
    void CreateVkImageView(VkFormat Format, VkImageAspectFlags AspectFlags);
    const VkSampler& GetImageSampler();
    const VkDeviceMemory& GetMemory();
    uint64_t GetMemorySize() const;

private:
    Image(bool SwapChainImg = false);

    void CreateSampler();

private:
    VkImage image_;
    VkImageView image_view_;
    VkDeviceMemory image_memory_;
    VkSampler sampler_;
    uint64_t memory_size_;
    bool swap_chain_img_;
};
}  // namespace toystation
