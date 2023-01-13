#pragma once
#include "VkContext.h"

namespace toystation {

VkImageCreateInfo MakeImage2DCreateInfo(
    const VkExtent2D& size, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM,
    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, bool mipmaps = false);

VkImageViewCreateInfo MakeImage2DViewCreateInfo(VkImage image,
    VkImageAspectFlags flags, VkFormat format = VK_FORMAT_R8G8B8A8_UNORM);
}  // namespace toystation