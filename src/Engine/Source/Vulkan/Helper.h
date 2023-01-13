#pragma once
#include "VkContext.h"
namespace toystation {
template <typename T>
void ZeroVKStruct(T& vk_struct,
                  VkStructureType type = VK_STRUCTURE_TYPE_APPLICATION_INFO) {
    memset(&vk_struct, 0, sizeof(vk_struct));
    vk_struct.sType = type;
}

}  // namespace toystation