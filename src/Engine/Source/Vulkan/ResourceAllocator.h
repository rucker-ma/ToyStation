#pragma once
#include "StagingMemoryManager.h"

namespace toystation {

struct Buffer {
    VkBuffer buffer = nullptr;
    MemHandle handle = nullptr;
};
struct RHIImage {
    VkImage image = nullptr;
    MemHandle handle = nullptr;
};
struct Texture {
    VkImage image = nullptr;
    MemHandle handle = nullptr;
    VkDescriptorImageInfo descriptor{};
};

class VkResourceAllocator {
public:
    VkResourceAllocator() = default;
    VkResourceAllocator(const VkResourceAllocator&) = delete;
    VkResourceAllocator& operator=(const VkResourceAllocator&) = delete;

    VkResourceAllocator(VkResourceAllocator&&) = delete;
    VkResourceAllocator& operator=(VkResourceAllocator&&) = delete;

    VkResourceAllocator(
        VkDevice device, VkPhysicalDevice physical_device,
        VkMemoryAllocator* mem_alloc,
        VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);
    virtual ~VkResourceAllocator();

    void Init(VkDevice device, VkPhysicalDevice physical_device,
              VkMemoryAllocator* mem_alloc,
              VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);
    void DeInit();
    virtual VkMemoryAllocator* GetMemoryAllocator() { return mem_alloc_; }

    virtual Buffer CreateBuffer(
        const VkBufferCreateInfo& info,
        VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        void* memory_export = nullptr);
    Buffer CreateBuffer(
        VkDeviceSize size = 0, VkBufferUsageFlags usage = VkBufferUsageFlags(),
        VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    Buffer CreateExternalBuffer(VkDeviceSize size, VkBufferUsageFlags usage = VkBufferUsageFlags(),
                                VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Buffer CreateBuffer(
        const VkCommandBuffer& cmd_buf, const VkDeviceSize& size,
        const void* data, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    template <typename T>
    Buffer CreateBuffer(const VkCommandBuffer& cmd_buf,
                        VkBufferUsageFlags usage, const std::vector<T>& data,
                        VkMemoryPropertyFlags mem_property =
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
        return CreateBuffer(cmd_buf, sizeof(T) * data.size(), data.data(),
                            usage, mem_property);
    }

    RHIImage CreateImage(
        const VkImageCreateInfo& info,
        VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    RHIImage CreateImage(
        const VkCommandBuffer& cmd_buffer, size_t size, const void* data,
        const VkImageCreateInfo& info,
        VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    Texture CreateTexture(const RHIImage& image,
                          const VkImageViewCreateInfo& imageview_create_info);
    Texture CreateTexture(const RHIImage& image,
                          const VkImageViewCreateInfo& imageview_create_info,
                          const VkSamplerCreateInfo& sampler_create_info);

    Texture CreateTexture(
        const VkCommandBuffer& cmd_buffer, size_t size, const void* data,
        const VkImageCreateInfo& info, VkSamplerCreateInfo& sampler_create_info,
        VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        bool is_cube = false);

    void Destroy(Buffer& buffer);
    void Destroy(RHIImage& image);
    void Destroy(Texture& texture);

    void* Map(const Buffer& buffer);
    void UnMap(const Buffer& buffer);

    void* Map(const RHIImage& image);
    void UnMap(const RHIImage& image);

protected:
    virtual void CreateBufferEx(const VkBufferCreateInfo& info,
                                VkBuffer* buffer);
    virtual void CreateImageEx(const VkImageCreateInfo& info, VkImage* image);

private:
    VkDevice device_ = nullptr;
    VkPhysicalDevice physical_device_ = nullptr;
    VkPhysicalDeviceMemoryProperties memory_properties_{};
    VkMemoryAllocator* mem_alloc_ = nullptr;
    std::unique_ptr<StagingMemoryManager> staging_;
};

class DedicatedResourceAllocator : public VkResourceAllocator {
public:
    DedicatedResourceAllocator() = default;
    DedicatedResourceAllocator(
        VkDevice device, VkPhysicalDevice physical_device,
        VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);
    virtual ~DedicatedResourceAllocator();
    void Init(VkContext* ctx,
              VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);

    void Init(VkDevice device, VkPhysicalDevice physical_device,
              VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);
    // Provided such that ResourceAllocatorDedicated, ResourceAllocatorDma and
    // ResourceAllocatorVma all have the same interface
    void Init(VkInstance instance, VkDevice device,
              VkPhysicalDevice physical_device,
              VkDeviceSize staging_block_size = DEFAULT_STAGING_BLOCKSIZE);

    void DeInit();

private:
    std::unique_ptr<VkMemoryAllocator> mem_alloc_;
};

}  // namespace toystation
