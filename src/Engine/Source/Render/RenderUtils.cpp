#include "RenderUtils.h"
namespace toystation {
RenderUtils::RenderUtils(VkCommandPool Pool) : command_pool_(Pool) {}
void RenderUtils::CopyBufferToImage(VkBuffer Buffer, VkImage Image,
                                    uint32_t Width, uint32_t Height) {
    VkCommandBuffer command_buffer = BeginSingletimeCommand();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageSubresource.mipLevel = 0;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {Width, Height, 1};
    vkCmdCopyBufferToImage(command_buffer, Buffer, Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingletimeCommand(command_buffer);
}
void RenderUtils::CopyBufferToGPU(void* Data, VkDeviceSize DataSize,
                                  VkBuffer& Buffer, VkDeviceMemory& Memory,
                                  VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties) {
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    CreateGPUBuffer(DataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    staging_buffer, staging_buffer_memory);
    void* data;
    vkMapMemory(VulkanDevice, staging_buffer_memory, 0, DataSize, 0, &data);
    memcpy(data, Data, DataSize);
    vkUnmapMemory(VulkanDevice, staging_buffer_memory);
    CreateGPUBuffer(DataSize, usage, properties, Buffer, Memory);
    // copy
    VkCommandBuffer command_buffer = BeginSingletimeCommand();
    VkBufferCopy copy_region{};
    copy_region.size = DataSize;
    vkCmdCopyBuffer(command_buffer, staging_buffer, Buffer, 1, &copy_region);
    EndSingletimeCommand(command_buffer);

    vkDestroyBuffer(VulkanDevice, staging_buffer, nullptr);
    vkFreeMemory(VulkanDevice, staging_buffer_memory, nullptr);
}

void RenderUtils::CreateGPUBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage,
                                  VkMemoryPropertyFlags Properties,
                                  VkBuffer& Buffer,
                                  VkDeviceMemory& BufferMemory) {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = Size;
    buffer_info.usage = Usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(VulkanDevice, &buffer_info, nullptr, &Buffer) !=
        VK_SUCCESS) {
    }
    VkMemoryRequirements mem_requires;
    vkGetBufferMemoryRequirements(VulkanDevice, Buffer, &mem_requires);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requires.size;

    alloc_info.memoryTypeIndex = VulkanContext::Instance().FindMemoryType(
        mem_requires.memoryTypeBits, Properties);
    if (vkAllocateMemory(VulkanDevice, &alloc_info, nullptr, &BufferMemory) !=
        VK_SUCCESS) {
    }
    vkBindBufferMemory(VulkanDevice, Buffer, BufferMemory, 0);
}

void RenderUtils::TransitionImageLayout(VkImage Image, VkImageLayout OldLayout,
                                        VkImageLayout NewLayout,
                                        VkAccessFlags OldAccessMask,
                                        VkAccessFlags NewAccessMask) {
    VkCommandBuffer command_buffer = BeginSingletimeCommand();
    TransitionImageLayoutWithCommandBuffer(command_buffer, Image, OldLayout,
                                           NewLayout, OldAccessMask,
                                           NewAccessMask);
    EndSingletimeCommand(command_buffer);
}

void RenderUtils::TransitionImageLayoutWithCommandBuffer(
    VkCommandBuffer CommandBuffer, VkImage Image, VkImageLayout OldLayout,
    VkImageLayout NewLayout, VkAccessFlags OldAccessMask,
    VkAccessFlags NewAccessMask) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = OldLayout;
    barrier.newLayout = NewLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = Image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = OldAccessMask;
    barrier.dstAccessMask = NewAccessMask;

    VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dest_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    // if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout ==
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    //{
    //     barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //     source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    //     dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    // }
    // else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout
    // == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    //{
    //     barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    //     barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    //     source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    //     dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    // }else
    //{
    //     barrier.srcAccessMask = OldAccessMask;
    //     barrier.dstAccessMask = NewAccessMask;
    // }

    vkCmdPipelineBarrier(CommandBuffer, source_stage, dest_stage, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);
}

VkCommandBuffer RenderUtils::BeginSingletimeCommand() {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;
    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(VulkanDevice, &alloc_info, &command_buffer);
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void RenderUtils::EndSingletimeCommand(VkCommandBuffer CommandBuffer) {
    vkEndCommandBuffer(CommandBuffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &CommandBuffer;
    vkQueueSubmit(VulkanGraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(VulkanGraphicsQueue);
    vkFreeCommandBuffers(VulkanDevice, command_pool_, 1, &CommandBuffer);
}
}  // namespace toystation
