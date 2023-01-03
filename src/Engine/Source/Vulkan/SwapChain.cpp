#include "SwapChain.h"

#include "Helper.h"

#define SURFACE_DEFAULT_WIDTH 1920
#define SURFACE_DEFAULT_HEIGHT 1080

namespace toystation {

VkFormat SwapChainBase::GetFormat() const { return surface_format_; }

VkRect2D* SwapChainBase::GetScissor() const {
    return const_cast<VkRect2D*>(&scissor_);
}

VkViewport* SwapChainBase::GetViewport() const {
    return const_cast<VkViewport*>(&viewport_);
}

bool SwapChain::Init(VkContext* ctx, VkQueue queue, uint32_t queue_family_index,
                     VkSurfaceKHR surface, VkFormat format,
                     VkImageUsageFlags image_flags) {
    ctx_ = ctx;
    surface_ = surface;
    image_usage_ = image_flags;
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice(ctx), surface_,
                                         &count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice(ctx), surface_,
                                         &count, surface_formats.data());

    surface_format_ = VK_FORMAT_B8G8R8A8_UNORM;
    surface_color_ = surface_formats[0].colorSpace;
    for (auto iter = surface_formats.begin(); iter != surface_formats.end();
         iter++) {
        if ((*iter).format == format) {
            surface_format_ = format;
            surface_color_ = (*iter).colorSpace;
            return true;
        }
    }
    return false;
}

Size2d SwapChain::Update(int width, int height) {
    VkSwapchainKHR old_swapchain = swapchain_;

    // TODO: add wait idle

    VkSurfaceCapabilitiesKHR surface_capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice(ctx_), surface_,
                                              &surface_capabilities);
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice(ctx_), surface_,
                                              &count, nullptr);
    std::vector<VkPresentModeKHR> present_modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice(ctx_), surface_,
                                              &count, present_modes.data());
    VkExtent2D swapchain_extent;
    if (surface_capabilities.currentExtent.width == (uint32_t)-1) {
        swapchain_extent.width = width;
        swapchain_extent.height = height;

    } else {
        swapchain_extent = surface_capabilities.currentExtent;
    }
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto&& mode : present_modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = mode;
            break;
        }
        if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            present_mode = mode;
        }
    }
    uint32_t desired_num_swapchain_images =
        surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 &&
        desired_num_swapchain_images > surface_capabilities.maxImageCount) {
        desired_num_swapchain_images = surface_capabilities.maxImageCount;
    }
    VkSurfaceTransformFlagBitsKHR pre_transform =
        surface_capabilities.currentTransform;
    if (surface_capabilities.supportedTransforms &
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    VkSwapchainCreateInfoKHR swapchain_create_info;
    ZeroVKStruct(swapchain_create_info,
                 VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
    swapchain_create_info.surface = surface_;
    swapchain_create_info.minImageCount = desired_num_swapchain_images;
    swapchain_create_info.imageFormat = surface_format_;
    swapchain_create_info.imageColorSpace = surface_color_;
    swapchain_create_info.imageUsage = image_usage_;
    swapchain_create_info.preTransform = pre_transform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 1;
    // TODO:set queue family
    // swapchain_create_info.pQueueFamilyIndices = ;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.oldSwapchain = old_swapchain;
    swapchain_create_info.clipped = VkBool32(true);
    vkCreateSwapchainKHR(ctx_->GetDevice(), &swapchain_create_info, nullptr,
                         &swapchain_);

    if (old_swapchain != nullptr) {
    }
    return Size2d{(int)swapchain_extent.width, (int)swapchain_extent.height};
}

SwapChainOffline::SwapChainOffline() : SwapChainBase() {
    surface_format_ = VK_FORMAT_B8G8R8A8_UNORM;
    scissor_ = {0};
    scissor_.extent.width = SURFACE_DEFAULT_WIDTH;
    scissor_.extent.height = SURFACE_DEFAULT_HEIGHT;
    viewport_ = {0, 0, SURFACE_DEFAULT_WIDTH, SURFACE_DEFAULT_HEIGHT, 0, 1};
}
}  // namespace toystation