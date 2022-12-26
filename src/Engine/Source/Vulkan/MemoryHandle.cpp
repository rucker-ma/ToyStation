#include "MemoryHandle.h"
namespace TSEngine {
MemoryAllocateInfo::MemoryAllocateInfo(const VkMemoryRequirements& mem_reqs,
                                       VkMemoryPropertyFlags mem_props,
                                       bool is_tiling_optimal) {
    mem_reqs_ = mem_reqs;
    mem_props_ = mem_props;
    is_tiling_optimal_ = is_tiling_optimal;
}

MemoryAllocateInfo::MemoryAllocateInfo(VkDevice device, VkBuffer buffer,
                                       VkMemoryPropertyFlags mem_props)
    : mem_props_{mem_props} {
    VkBufferMemoryRequirementsInfo2 buffer_reqs = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2, nullptr, buffer};
    VkMemoryDedicatedRequirements dedicated_reqs = {
        VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS};
    VkMemoryRequirements2 mem_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
                                      &dedicated_reqs};
    vkGetBufferMemoryRequirements2(device, &buffer_reqs, &mem_reqs);
    mem_reqs_ = mem_reqs.memoryRequirements;
    if (dedicated_reqs.requiresDedicatedAllocation) {
        SetBuffer(buffer);
    }
    SetTilingOptimal(false);
}
MemoryAllocateInfo::MemoryAllocateInfo(VkDevice device, VkImage image,
                                       VkMemoryPropertyFlags mem_props)
    : mem_props_{mem_props} {
    VkImageMemoryRequirementsInfo2 image_reqs = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2, nullptr, image};
    VkMemoryDedicatedRequirements dedicated_reqs = {
        VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS};
    VkMemoryRequirements2 mem_reqs = {VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2,
                                      &dedicated_reqs};

    vkGetImageMemoryRequirements2(device, &image_reqs, &mem_reqs);

    mem_reqs_ = mem_reqs.memoryRequirements;

    if (dedicated_reqs.requiresDedicatedAllocation) {
        SetImage(image);
    }

    SetTilingOptimal(true);
}
MemoryAllocateInfo& MemoryAllocateInfo::SetImage(VkImage image) {
    assert(!buffer_);
    image_ = image;
    return *this;
}
MemoryAllocateInfo& MemoryAllocateInfo::SetBuffer(VkBuffer buffer) {
    assert(!image_);
    buffer_ = buffer;
    return *this;
}
MemoryAllocateInfo& MemoryAllocateInfo::SetAllocateFlags(
    VkMemoryAllocateFlags flags) {
    allocate_flags_ |= flags;
    return *this;
}
MemoryAllocateInfo& MemoryAllocateInfo::SetDeviceMask(uint32_t mask) {
    device_mask_ = mask;
    return *this;
}

MemoryAllocateInfo& MemoryAllocateInfo::SetTilingOptimal(bool tiling_optimal) {
    is_tiling_optimal_ = tiling_optimal;
    return *this;
}

MemoryAllocateInfo& MemoryAllocateInfo::SetDebugName(const std::string& name) {
    debug_name_ = name;
    return *this;
}
MemoryAllocateInfo& MemoryAllocateInfo::SetPriority(const float priority) {
    priority_ = priority;
    return *this;
}
VkImage MemoryAllocateInfo::GetImage() const { return image_; }
VkBuffer MemoryAllocateInfo::GetBuffer() const { return buffer_; }
VkMemoryAllocateFlags MemoryAllocateInfo::GetAllocateFlags() const {
    return allocate_flags_;
}
uint32_t MemoryAllocateInfo::GetDeviceMask() const { return device_mask_; }
const VkMemoryRequirements& MemoryAllocateInfo::GetMemoryRequirements() const {
    return mem_reqs_;
}
const VkMemoryPropertyFlags& MemoryAllocateInfo::GetMemoryProperties() const {
    return mem_props_;
}
std::string MemoryAllocateInfo::GetDebugName() const { return debug_name_; }
float MemoryAllocateInfo::GetPriority() const { return priority_; }

bool MemoryAllocateUtils::FillBakedAllocateInfo(
    const VkPhysicalDeviceMemoryProperties& mem_props,
    const MemoryAllocateInfo& info, BakedAllocateInfo& baked) {
    baked.mem_alloc_info.allocationSize = info.GetMemoryRequirements().size;
    baked.mem_alloc_info.memoryTypeIndex =
        GetMemoryType(mem_props, info.GetMemoryRequirements().memoryTypeBits,
                      info.GetMemoryProperties());

    // Put it last in the chain, so we can directly pass it into the
    // DeviceMemoryAllocator::alloc function
    if (info.GetBuffer() || info.GetImage()) {
        baked.dedicated_info.pNext = baked.mem_alloc_info.pNext;
        baked.mem_alloc_info.pNext = &baked.dedicated_info;

        baked.dedicated_info.buffer = info.GetBuffer();
        baked.dedicated_info.image = info.GetImage();
    }

    //   if(info.getExportable())
    //   {
    //     baked.exportInfo.pNext   = baked.memAllocInfo.pNext;
    //     baked.memAllocInfo.pNext = &baked.exportInfo;
    // #ifdef WIN32
    //     baked.exportInfo.handleTypes =
    //     VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    // #else
    //     baked.exportInfo.handleTypes =
    //     VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
    // #endif
    //   }

    if (info.GetDeviceMask() || info.GetAllocateFlags()) {
        baked.flags_info.pNext = baked.mem_alloc_info.pNext;
        baked.mem_alloc_info.pNext = &baked.flags_info;

        baked.flags_info.flags = info.GetAllocateFlags();
        baked.flags_info.deviceMask = info.GetDeviceMask();

        if (baked.flags_info.deviceMask) {
            baked.flags_info.flags |= VK_MEMORY_ALLOCATE_DEVICE_MASK_BIT;
        }
    }

    return true;
}
uint32_t MemoryAllocateUtils::GetMemoryType(
    const VkPhysicalDeviceMemoryProperties& memory_properties,
    uint32_t type_bits, const VkMemoryPropertyFlags& properties) {
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if (((type_bits & (1 << i)) > 0) &&
            (memory_properties.memoryTypes[i].propertyFlags &  // NOLINT
             properties) == properties) {
            return i;
        }
    }
    assert(0);
    return ~0U;
}
}  // namespace TSEngine
