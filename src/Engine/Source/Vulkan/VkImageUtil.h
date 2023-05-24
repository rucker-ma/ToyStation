#pragma once
#include "VkHelper.h"
namespace toystation {
class VkImageUtil {
public:
    static void CmdBarrierImageLayout(
        VkCommandBuffer cmd_buffer, VkImage image,
        VkImageLayout old_image_layout, VkImageLayout new_image_layout,
        const VkImageSubresourceRange& subresource_range);
    static VkAccessFlags AccessFlagsFromImageLayout(VkImageLayout layout);
    static VkPipelineStageFlags PipelineStageForLayout(VkImageLayout layout);
};
}  // namespace toystation