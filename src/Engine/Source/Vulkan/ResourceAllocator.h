#pragma once
#include "StagingMemoryManager.h"
#include "CommandPool.h"
#include <ktxvulkan.h>

namespace toystation {

struct RHIBuffer {
    VkBuffer buffer = nullptr;
    MemHandle handle = nullptr;
};
struct RHIImage {
    VkImage image = nullptr;
    MemHandle handle = nullptr;
};
struct RHITexture {
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

    virtual RHIBuffer CreateBuffer(
        const VkBufferCreateInfo& info,
        VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    RHIBuffer CreateBuffer(
        VkDeviceSize size = 0, VkBufferUsageFlags usage = VkBufferUsageFlags(),
        VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    RHIBuffer CreateExternalBuffer(VkDeviceSize size, VkBufferUsageFlags usage = VkBufferUsageFlags(),
                                VkMemoryPropertyFlags mem_usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    RHIBuffer CreateBuffer(
        const VkCommandBuffer& cmd_buf, const VkDeviceSize& size,
        const void* data, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags mem_props = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    template <typename T>
    RHIBuffer CreateBuffer(const VkCommandBuffer& cmd_buf,
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

    RHITexture CreateTexture(const RHIImage& image,
                          const VkImageViewCreateInfo& imageview_create_info,
                             bool create_sampler =false);
    RHITexture CreateTexture(const RHIImage& image,
                          const VkImageViewCreateInfo& imageview_create_info,
                          const VkSamplerCreateInfo& sampler_create_info);

    RHITexture CreateTexture(
        const VkCommandBuffer& cmd_buffer, size_t size, const void* data,
        const VkImageCreateInfo& info, VkSamplerCreateInfo& sampler_create_info,
        VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        bool is_cube = false);

    void Destroy(RHIBuffer& RHIBuffer);
    void Destroy(RHIImage& image);
    void Destroy(RHITexture& RHITexture);

    void* Map(const RHIBuffer& RHIBuffer);
    void UnMap(const RHIBuffer& RHIBuffer);

    void* Map(const RHIImage& image);
    void UnMap(const RHIImage& image);

    RHITexture LoadKTXFileAsTexture(std::string path,VkQueue queue,std::shared_ptr<CommandPool> pool);

protected:
    virtual void CreateBufferEx(const VkBufferCreateInfo& info,
                                VkBuffer* RHIBuffer,bool with_external=false);
    virtual void CreateImageEx(const VkImageCreateInfo& info, VkImage* image);

    RHITexture CreateCubemap(ktxTexture* ktxtexture,std::shared_ptr<CommandPool> pool);
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
