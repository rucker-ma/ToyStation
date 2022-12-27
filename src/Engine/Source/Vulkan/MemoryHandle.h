#pragma once
#include "Base/Macro.h"
#include "Helper.h"
#include "Render/VulkanContext.h"

namespace toystation {

class MemoryHandleBase;
using MemHandle = MemoryHandleBase*;

class MemoryAllocateInfo {
public:
    MemoryAllocateInfo(const VkMemoryRequirements& mem_reqs,
                       VkMemoryPropertyFlags mem_props =
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                       bool is_tiling_optimal = false);
    MemoryAllocateInfo(
        VkDevice device, VkBuffer buffer,
        VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    MemoryAllocateInfo(
        VkDevice device, VkImage image,
        VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    // MemoryAllocateInfo& SetMemoryProperties(VkMemoryPropertyFlags flags);
    // MemoryAllocateInfo& SetMemoryRequirements(
    //     VkMemoryRequirements requirements);
    MemoryAllocateInfo& SetTilingOptimal(bool tiling_optimal);
    MemoryAllocateInfo& SetImage(VkImage image);
    MemoryAllocateInfo& SetBuffer(VkBuffer buffer);
    MemoryAllocateInfo& SetAllocateFlags(VkMemoryAllocateFlags flags);
    MemoryAllocateInfo& SetDeviceMask(uint32_t mask);
    MemoryAllocateInfo& SetDebugName(const std::string& name);
    MemoryAllocateInfo& SetPriority(const float priority = 1.0);

    VkImage GetImage() const;
    VkBuffer GetBuffer() const;
    VkMemoryAllocateFlags GetAllocateFlags() const;
    uint32_t GetDeviceMask() const;
    const VkMemoryRequirements& GetMemoryRequirements() const;
    const VkMemoryPropertyFlags& GetMemoryProperties() const;
    std::string GetDebugName() const;
    float GetPriority() const;

private:
    VkBuffer buffer_{nullptr};
    VkImage image_{nullptr};
    VkMemoryAllocateFlags allocate_flags_{0};
    uint32_t device_mask_{0};
    VkMemoryRequirements mem_reqs_;
    VkMemoryPropertyFlags mem_props_;
    float priority_{1.0};
    std::string debug_name_;
    bool is_tiling_optimal_;
};

struct BakedAllocateInfo {
    VkMemoryAllocateInfo mem_alloc_info;
    VkMemoryAllocateFlagsInfo flags_info;
    VkMemoryDedicatedAllocateInfo dedicated_info;
    VkExportMemoryAllocateInfo export_info;
    BakedAllocateInfo(BakedAllocateInfo&& other) = delete;
    BakedAllocateInfo operator=(BakedAllocateInfo&& other) = delete;
    BakedAllocateInfo(const BakedAllocateInfo&) = delete;
    BakedAllocateInfo operator=(const BakedAllocateInfo) = delete;
    BakedAllocateInfo() {
        ZeroVKStruct(mem_alloc_info, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
        ZeroVKStruct(flags_info, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO);
        ZeroVKStruct(dedicated_info,
                     VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO);
        ZeroVKStruct(export_info,
                     VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO);
    }
};

class MemoryAllocateUtils {
public:
    static bool FillBakedAllocateInfo(
        const VkPhysicalDeviceMemoryProperties& mem_props,
        const MemoryAllocateInfo& info, BakedAllocateInfo& baked);

    static uint32_t GetMemoryType(
        const VkPhysicalDeviceMemoryProperties& memory_properties,
        uint32_t type_bits, const VkMemoryPropertyFlags& properties);
};

class MemoryHandleBase {
public:
    MemoryHandleBase() = default;
    MemoryHandleBase(const MemoryHandleBase& mem) = delete;
    MemoryHandleBase& operator=(const MemoryHandleBase& mem) = delete;
    MemoryHandleBase(MemoryHandleBase&& mem) = delete;
    MemoryHandleBase& operator=(MemoryHandleBase&& mem) = delete;
    virtual ~MemoryHandleBase() = default;
};
}  // namespace toystation