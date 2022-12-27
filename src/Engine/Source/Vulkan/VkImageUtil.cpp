#include "VkImageUtil.h"

void toystation::VkImageUtil::CmdBarrierImageLayout(
    VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout old_image_layout,
    VkImageLayout new_image_layout,
    const VkImageSubresourceRange& subresource_range) {
    VkImageMemoryBarrier image_memory_barrier;

    ZeroVKStruct(image_memory_barrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
    image_memory_barrier.oldLayout = old_image_layout;
    image_memory_barrier.newLayout = new_image_layout;
    image_memory_barrier.image = image;
    image_memory_barrier.subresourceRange = subresource_range;
    image_memory_barrier.srcAccessMask =
        AccessFlagsFromImageLayout(old_image_layout);
    image_memory_barrier.dstAccessMask =
        AccessFlagsFromImageLayout(new_image_layout);
    // Fix for a validation issue - should be needed when VkImage sharing mode
    // is VK_SHARING_MODE_EXCLUSIVE and the values of srcQueueFamilyIndex and
    // dstQueueFamilyIndex are equal, no ownership transfer is performed, and
    // the barrier operates as if they were both set to VK_QUEUE_FAMILY_IGNORED.
    image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    VkPipelineStageFlags src_stage_mask =
        PipelineStageForLayout(old_image_layout);
    VkPipelineStageFlags dst_stage_mask =
        PipelineStageForLayout(new_image_layout);
    vkCmdPipelineBarrier(cmd_buffer, src_stage_mask, dst_stage_mask, 0, 0,
                         nullptr, 0, nullptr, 1, &image_memory_barrier);
}

VkAccessFlags toystation::VkImageUtil::AccessFlagsFromImageLayout(
    VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_ACCESS_HOST_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return VK_ACCESS_TRANSFER_WRITE_BIT;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_ACCESS_TRANSFER_READ_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_ACCESS_SHADER_READ_BIT;
        default:
            return VkAccessFlags();
    }
}

VkPipelineStageFlags toystation::VkImageUtil::PipelineStageForLayout(
    VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return VK_PIPELINE_STAGE_TRANSFER_BIT;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // We do this to allow
                                                        // queue other than
                                                        // graphic return
                                                        // VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;  // We do this to allow
                                                        // queue other than
                                                        // graphic return
                                                        // VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            return VK_PIPELINE_STAGE_HOST_BIT;
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        default:
            return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
}
