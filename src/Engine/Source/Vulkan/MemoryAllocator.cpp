#include "MemoryAllocator.h"

namespace TSEngine {

class DedicatedMemoryHandle : public MemoryHandleBase {
public:
    VkDeviceMemory GetMemory() const { return memory_; }
    VkDeviceSize GetSize() const { return size_; }

private:
    friend class VkMemoryAllocator;
    DedicatedMemoryHandle(VkDeviceMemory memory, VkDeviceSize size)
        : memory_(memory), size_(size) {}
    VkDeviceMemory memory_;
    VkDeviceSize size_;
};

DedicatedMemoryHandle* CastDedicatedMemoryHandle(MemHandle mem_handle) {
    if (!mem_handle) {
        return nullptr;
    }
#ifndef NDEBUG
    auto dedicated_handle = static_cast<DedicatedMemoryHandle*>(mem_handle);
#else
    auto dedicated_handle = dynamic_cast<DedicatedMemoryHandle*>(mem_handle);
    assert(dedicated_handle);
#endif

    return dedicated_handle;
}

void VkMemoryAllocator::Init(VkDevice device,
                             VkPhysicalDevice physical_device) {
    device_ = device;
    physical_device_ = physical_device;
    vkGetPhysicalDeviceMemoryProperties(physical_device_,
                                        &physical_memory_properties_);
}

void VkMemoryAllocator::DeInit() { device_ = nullptr; }

MemHandle VkMemoryAllocator::AllocMemory(const MemoryAllocateInfo& info) {
    MemoryAllocateInfo local_info(info);
    local_info.SetAllocateFlags(info.GetAllocateFlags() | flags_);
    BakedAllocateInfo baked_info;
    MemoryAllocateUtils::FillBakedAllocateInfo(physical_memory_properties_,
                                               local_info, baked_info);
    VkDeviceMemory memory = nullptr;
    VkResult result =
        vkAllocateMemory(device_, &baked_info.mem_alloc_info, nullptr, &memory);
    if (result != VK_SUCCESS) {
        LogError("Allocate Memory Error");
        return nullptr;
    }
    auto* dedicated_mem_handle = new DedicatedMemoryHandle(
        memory, baked_info.mem_alloc_info.allocationSize);
    return dedicated_mem_handle;
}
void VkMemoryAllocator::FreeMemory(MemHandle mem_handle) {
    if (!mem_handle) {
        return;
    }
    auto* dedicated_handle = CastDedicatedMemoryHandle(mem_handle);
    vkFreeMemory(device_, dedicated_handle->GetMemory(), nullptr);
    delete dedicated_handle;
}
VkMemoryAllocator::MemoryInfo VkMemoryAllocator::GetMemoryInfo(
    MemHandle mem_handle) const {
    auto* dedicated_handle = CastDedicatedMemoryHandle(mem_handle);

    return MemoryInfo{dedicated_handle->GetMemory(), 0,
                      dedicated_handle->GetSize()};
}
void* VkMemoryAllocator::Map(MemHandle mem_handle, VkDeviceSize offset,
                             VkDeviceSize size) {
    auto* dedicated_handle = CastDedicatedMemoryHandle(mem_handle);
    void* ptr = nullptr;
    VkResult result = vkMapMemory(device_, dedicated_handle->GetMemory(),
                                  offset, size, 0 /*VkMemoryFlags*/, &ptr);

    return ptr;
}
void VkMemoryAllocator::UnMap(MemHandle mem_handle) {
    auto* dedicated_handle = CastDedicatedMemoryHandle(mem_handle);
    vkUnmapMemory(device_, dedicated_handle->GetMemory());
}
void VkMemoryAllocator::SetAllocateFlags(VkMemoryAllocateFlags flags) {
    flags_ = flags;
}
VkDevice VkMemoryAllocator::GetDevice() const { return device_; }
VkPhysicalDevice VkMemoryAllocator::GetPhysicalDevice() const {
    return physical_device_;
}
}  // namespace TSEngine