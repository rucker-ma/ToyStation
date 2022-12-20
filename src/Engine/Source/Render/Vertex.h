#pragma once
#include <array>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderUtils.h"
#include "VulkanContext.h"

namespace TSEngine {

class Vertex {
public:
    static VkVertexInputBindingDescription GetBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3>
    GetAttributeDescriptions();

    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

extern const std::vector<Vertex> vertices;
extern const std::vector<uint16_t> indices;

template <class T>
class GPUBuffer {
public:
    GPUBuffer(RenderUtils* Utils, const std::vector<T>& Buffer,
              VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    GPUBuffer(RenderUtils* Utils, VkBufferUsageFlags usage,
              VkMemoryPropertyFlags properties);
    ~GPUBuffer();
    const VkBuffer& GetBuffer();
    const VkDeviceMemory& GetMemory();

private:
    VkBuffer buffer_;
    VkDeviceMemory memory_;
};
template <class T>
inline GPUBuffer<T>::GPUBuffer(RenderUtils* Utils, const std::vector<T>& Buffer,
                               VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties) {
    VkDeviceSize Size = Buffer.size() * sizeof(T);
    Utils->CopyBufferToGPU(const_cast<T*>(Buffer.data()), Size, buffer_,
                           memory_, usage, properties);
}

template <class T>
inline GPUBuffer<T>::GPUBuffer(RenderUtils* Utils, VkBufferUsageFlags usage,
                               VkMemoryPropertyFlags properties) {
    VkDeviceSize buffersize = sizeof(T);
    Utils->CreateGPUBuffer(buffersize, usage, properties, buffer_, memory_);
}

template <class T>
inline GPUBuffer<T>::~GPUBuffer() {
    vkDestroyBuffer(VulkanDevice, buffer_, nullptr);
    vkFreeMemory(VulkanDevice, memory_, nullptr);
}

template <class T>
inline const VkBuffer& GPUBuffer<T>::GetBuffer() {
    return buffer_;
}

template <class T>
inline const VkDeviceMemory& GPUBuffer<T>::GetMemory() {
    return memory_;
}

}  // namespace TSEngine
