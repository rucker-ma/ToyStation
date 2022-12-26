#pragma once
#include "Vulkan/MemoryHandle.h"

namespace TSEngine {

class VkMemoryAllocator {
public:
    struct MemoryInfo {
        VkDeviceMemory memory;
        VkDeviceSize offset;
        VkDeviceSize size;
    };

    void Init(VkDevice device, VkPhysicalDevice physical_device);
    void DeInit();
    MemHandle AllocMemory(const MemoryAllocateInfo& info);
    void FreeMemory(MemHandle mem_handle);
    MemoryInfo GetMemoryInfo(MemHandle mem_handle) const;
    void* Map(MemHandle mem_handle, VkDeviceSize offset = 0,
              VkDeviceSize size = VK_WHOLE_SIZE);
    void UnMap(MemHandle mem_handle);

    void SetAllocateFlags(VkMemoryAllocateFlags flags);

    VkDevice GetDevice() const;
    VkPhysicalDevice GetPhysicalDevice() const;

private:
    VkDevice device_{nullptr};
    VkPhysicalDevice physical_device_{nullptr};
    VkPhysicalDeviceMemoryProperties physical_memory_properties_;
    VkMemoryAllocateFlags flags_{0};
};

}  // namespace TSEngine