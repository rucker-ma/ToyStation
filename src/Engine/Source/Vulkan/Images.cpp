#include "Images.h"

namespace toystation {
VkImageCreateInfo MakeImage2DCreateInfo(const VkExtent2D &size, VkFormat format,
                                        VkImageUsageFlags usage,bool mipmaps) {
    VkImageCreateInfo create_info;
    ZeroVKStruct(create_info, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);

    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.extent.width = size.width;
    create_info.extent.height = size.height;
    create_info.extent.depth = 1;
    create_info.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    return create_info;
}
VkImageViewCreateInfo MakeImage2DViewCreateInfo(VkImage image,
                                                VkImageAspectFlags flags,
                                                VkFormat format) {
    VkImageViewCreateInfo create_info;
    ZeroVKStruct(create_info, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.subresourceRange.aspectMask = flags;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;
    return create_info;
}
}  // namespace toystation