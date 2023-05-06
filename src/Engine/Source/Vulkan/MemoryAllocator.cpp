#include "MemoryAllocator.h"
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif
namespace toystation {

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
// #ifndef NDEBUG
//     auto* dedicated_handle = dynamic_cast<DedicatedMemoryHandle*>(mem_handle);
// #else
    auto* dedicated_handle = dynamic_cast<DedicatedMemoryHandle*>(mem_handle);
    assert(dedicated_handle);
//#endif

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
MemHandle VkMemoryAllocator::AllocMemory(const VkMemoryAllocateInfo & info)
{
    VkDeviceMemory memory = nullptr;
    VkResult result = vkAllocateMemory(device_, &info, nullptr, &memory);
    if (result != VK_SUCCESS) {
        LogError("Allocate Memory Error");
        return nullptr;
    }
    auto* dedicated_mem_handle = new DedicatedMemoryHandle(
        memory, info.allocationSize);
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


VkDeviceSize MemHandleUtils::GetSize(MemHandle mem_handle){
    auto* dedicated_handle = CastDedicatedMemoryHandle(mem_handle);
    if(dedicated_handle){
        return dedicated_handle->GetSize();
    }
    LogError("Memory handle cast error");
    assert(dedicated_handle);
    return 0;
}
void* MemHandleUtils::GetExternalWin32Handle(VkDevice device,MemHandle mem_handle){
    auto* dedicated_handle = CastDedicatedMemoryHandle(mem_handle);
    HANDLE handle = 0;
    VkMemoryGetWin32HandleInfoKHR win32_handle_info;
    ZeroVKStruct(win32_handle_info,VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR);
    win32_handle_info.memory = dedicated_handle->GetMemory();
    win32_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;

    PFN_vkGetMemoryWin32HandleKHR fpGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(
        device,"vkGetMemoryWin32HandleKHR"
        );
    if(!fpGetMemoryWin32HandleKHR){
        LogError("Failed to retrieve vkGetMemoryWin32HandleKHR");
        return nullptr;
    }
    VkResult res = fpGetMemoryWin32HandleKHR(device,&win32_handle_info,&handle);
    if(res !=VK_SUCCESS){
        LogError("Get Memory win32 handle error, confirm memory is exportable");
        return nullptr;
    }
    return static_cast<void*>(handle);
}



}  // namespace toystation