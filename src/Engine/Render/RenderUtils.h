#pragma once
#include "VulkanContext.h"

namespace TSEngine {

    class RenderUtils
    {
    public:
        RenderUtils(VkCommandPool Pool);
        void CopyBufferToImage(VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height);
        void CopyBufferToGPU(void* Data, VkDeviceSize DataSize, VkBuffer& Buffer, VkDeviceMemory& Memory, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        void CreateGPUBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory);
        void TransitionImageLayout(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
        VkAccessFlags OldAccessMask,VkAccessFlags NewAccessMask);
        void TransitionImageLayoutWithCommandBuffer(VkCommandBuffer CommandBuffer,VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout,
     VkAccessFlags OldAccessMask,VkAccessFlags NewAccessMask);
        VkCommandBuffer BeginSingletimeCommand();
        void EndSingletimeCommand(VkCommandBuffer CommandBuffer);
    private:
        
         VkCommandPool command_pool_;
    };
}

