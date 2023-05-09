#pragma once
#include "Vulkan/MemoryHandle.h"

namespace toystation {

#define DEFAULT_STAGING_BLOCKSIZE (VkDeviceSize(64) * 1024 * 1024)

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

    MemHandle AllocMemory(const VkMemoryAllocateInfo & info);
    MemHandle WrapperMemory(VkDeviceMemory memory,VkDeviceSize size = 0);
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

class MemHandleUtils{
public:
    static VkDeviceSize GetSize(MemHandle mem_handle);
    static void* GetExternalWin32Handle(VkDevice device,MemHandle mem_handle);
};
}  // namespace toystation