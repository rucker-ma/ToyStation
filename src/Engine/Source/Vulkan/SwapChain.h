#pragma once
#include "Base/Container.h"
#include "ResourceAllocator.h"
#include "VkContext.h"

namespace toystation {

class SwapChainBase {
public:
    VkFormat GetFormat() const;

    virtual VkRect2D* GetScissor() const;
    virtual VkViewport* GetViewport() const;

protected:
    VkFormat surface_format_;
    VkRect2D scissor_;
    VkViewport viewport_;
};

class SwapChain : public SwapChainBase {
public:
    bool Init(VkContext* ctx, VkQueue queue, uint32_t queue_family_index,
              VkSurfaceKHR surface, VkFormat format = VK_FORMAT_B8G8R8A8_UNORM,
              VkImageUsageFlags image_flags =
                  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    Size2d Update(int width, int height);

private:
    VkContext* ctx_;
    VkSurfaceKHR surface_;
    VkImageUsageFlags image_usage_;
    VkColorSpaceKHR surface_color_;
    VkSwapchainKHR swapchain_;
};

class SwapChainOffline : public SwapChainBase {
public:
    SwapChainOffline();
    // VkViewport* GetViewport() const override;
    // VkRect2D* GetScissor() const override;

private:
    std::shared_ptr<DedicatedResourceAllocator> alloc_;
};
}  // namespace toystation