#pragma once
#include <optional>
#include <vector>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32


#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "Base/Macro.h"

#define VulkanInstance VulkanContext::Instance().VKInstance()
#define VulkanDevice VulkanContext::Instance().Device()
#define VulkanPhysicsDevice VulkanContext::Instance().PhysicalDevice()
#define VulkanCommandPool VulkanContext::Instance().CommandPool()
#define VulkanGraphicsQueue VulkanContext::Instance().GraphicsQueue()
#define VulkanPresentQueue VulkanContext::Instance().GraphicsQueue()
#define VulkanSurface VulkanContext::Instance().Surface()
namespace TSEngine
{
    class ImageFactory;
    class Image;

    struct VKSwapChainInfo {
        VkSwapchainKHR SwapChain;
        VkFormat SurfaceFormat;
        VkExtent2D Extend;
    };


    struct VkCtx
    {
        VkInstance instance;
        VkPhysicalDevice physice_device;
        VkDevice device;
        VkQueue queue;
        uint32_t graphics_queue_idx;
    };
    struct ImageInfo
    {
        unsigned long long image;
        unsigned long long image_view;
        unsigned long long memory;
        int width;
        int height;
        uint32_t layout;
        uint32_t tiling;
        uint32_t format;
        uint32_t usage_flags;
        uint64_t memory_size;
    };
    typedef void(*DebugFunction)(char*);
 
    struct VulkanQueueFamily
    {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
        bool has_value()
        {
            return graphics_family.has_value() && present_family.has_value();
        }
    };
    struct VulkanSwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };
    CLASS(SINGLETON)
    class TS_CPP_API VulkanContext
    {
    public:
        static VulkanContext& Instance();
        FUNCTION(CSHARP)
        void CreateInstance(DebugFunction func = nullptr,bool EnableDebug = true);
        FUNCTION(CSHARP)
        void CreateSurface(HWND hwnd);
        void CreatePhysicalDevice();
        void CreateLogicalDevice();
        FUNCTION(CSHARP)
        VkCtx GetContext();

        FUNCTION(CSHARP)
        void* GetDeviceProcAddr(char* name);
        FUNCTION(CSHARP)
        void* GetInstanceProcAddr(char* name);

        const VkInstance& VKInstance();
        const VkPhysicalDevice& PhysicalDevice();
        const VkDevice& Device();
        const VkQueue& GraphicsQueue();
        const VkQueue& PresentQueue();
        const VkSurfaceKHR& Surface();
        
        uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties);

        VulkanSwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice Device);
        VulkanQueueFamily FindQueueFamilies(VkPhysicalDevice phy_device);
    private:
        VulkanContext();
        std::vector<const char*> GetRequiredExtensions();
        void SetupDebugMessenger();
        bool ValidationLayerCheck();
        bool SuitableDevice(VkPhysicalDevice Device);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice Device);
        void SetupDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info_ext);
        
        void CreatePresentImage();
    private:
        VkInstance instance_;
        VkPhysicalDevice physical_device_;
        VkDevice device_;
        VkDebugUtilsMessengerEXT debug_utils_messenger_ext_;
        VkQueue present_queue_;
        VkQueue graphics_queue_;
        VkSurfaceKHR surface_;

        VulkanQueueFamily indices;
        std::vector<const char*> device_extensions_;
        std::vector<VkExtensionProperties> instance_extensions_;
    };
}
