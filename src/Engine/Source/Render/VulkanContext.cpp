#include "VulkanContext.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "ImageFactory.h"
#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define VK_RES_CHECK(res, ...)                                             \
    if (res != VK_SUCCESS)                                                 \
        throw std::runtime_error(                                          \
            std::string(__FILENAME__) + " : " + std::to_string(__LINE__) + \
            ":  vulkan return value error: " + std::to_string(res));

#define VK_PROCESS(func) \
    VkResult res = func; \
    VK_RES_CHECK(res)

namespace toystation {
DebugFunction csharp_output_;

static VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageServerity,
                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void* pUserData) {
    if (messageServerity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        if (csharp_output_) {
            csharp_output_((char*)pCallbackData->pMessage);
        }
    }
    // std::cout << "validation layer:" << pCallbackData->pMessage << std::endl;
    return false;
}

VulkanContext& VulkanContext::Instance() {
    static VulkanContext vk;
    return vk;
}

VulkanQueueFamily VulkanContext::FindQueueFamilies(
    VkPhysicalDevice phy_device) {
    VulkanQueueFamily queue_family_res;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phy_device, &queue_family_count,
                                             queue_families.data());
    int i = 0;

    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queue_family_res.graphics_family = i;
        }
        if (queue_family.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            queue_family_res.encode_family = i;
            std::cerr << "Get Support Encode Queue" << std::endl;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phy_device, i, surface_,
                                             &presentSupport);
        queue_family_res.present_family = i;
        i++;
    }
    return queue_family_res;
}

const VkInstance& VulkanContext::VKInstance() { return instance_; }

const VkPhysicalDevice& VulkanContext::PhysicalDevice() {
    return physical_device_;
}

const VkDevice& VulkanContext::Device() { return device_; }

// todo: get or set graphics queue when get device
const VkQueue& VulkanContext::GraphicsQueue() { return graphics_queue_; }

const VkQueue& VulkanContext::PresentQueue() { return present_queue_; }

const VkSurfaceKHR& VulkanContext::Surface() { return surface_; }

VulkanContext::VulkanContext()
    : instance_(nullptr),
      physical_device_(nullptr),
      device_(nullptr),
      debug_utils_messenger_ext_(nullptr),
      surface_(nullptr) {}

std::vector<const char*> VulkanContext::GetRequiredExtensions() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    instance_extensions_.resize(extensionCount);
    std::vector<const char*> enable_extensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                           instance_extensions_.data());
    for (auto& extension : instance_extensions_) {
        if (std::string(extension.extensionName) ==
            std::string("VK_KHR_surface")) {
            enable_extensions.push_back(extension.extensionName);
        }
#ifdef _WIN32
        if (std::string(extension.extensionName) ==
            std::string("VK_KHR_win32_surface")) {
            enable_extensions.push_back(extension.extensionName);
        }
#endif  // _WIN32
    }
    return enable_extensions;
}

void VulkanContext::CreateInstance(DebugFunction func, bool EnableDebug) {
    csharp_output_ = func;
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "toystation";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_MAKE_VERSION(1, 2, 0);
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;

    std::vector<const char*> extensions = GetRequiredExtensions();

    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    validationLayers.push_back("VK_LAYER_LUNARG_api_dump");

    // enable for debug
    if (EnableDebug) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        create_info.enabledExtensionCount =
            static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        create_info.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        create_info.ppEnabledLayerNames = validationLayers.data();

        // Init for VkDebugUtilsMessengerCreateInfoEXT
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_create_info{};
        SetupDebugMessengerCreateInfo(debug_utils_create_info);

        create_info.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debug_utils_create_info;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.pNext = nullptr;
    }
    VK_PROCESS(vkCreateInstance(&create_info, nullptr, &instance_))
    if (EnableDebug) {
        SetupDebugMessenger();
    }
}

void VulkanContext::SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT create_info_ext{};
    SetupDebugMessengerCreateInfo(create_info_ext);
    auto debug_utils_func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance_, "vkCreateDebugUtilsMessengerEXT");
    if (!debug_utils_func) {
        VK_RES_CHECK(VK_ERROR_EXTENSION_NOT_PRESENT)
    }
    VK_PROCESS(debug_utils_func(instance_, &create_info_ext, nullptr,
                                &debug_utils_messenger_ext_))
}

void VulkanContext::CreateSurface(HWND hwnd) {
    VkWin32SurfaceCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = hwnd;
    create_info.hinstance = GetModuleHandle(nullptr);
    VK_PROCESS(
        vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_));
}

void VulkanContext::CreatePhysicalDevice() {
    device_extensions_.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // for vulkan encode
    device_extensions_.push_back(
        VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME);
    device_extensions_.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    device_extensions_.push_back(VK_KHR_VIDEO_QUEUE_EXTENSION_NAME);
    device_extensions_.push_back(VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME);
    device_extensions_.push_back(VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME);

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
    if (device_count) {
        // todo: throw or log error message
    }
    physical_device_ = VK_NULL_HANDLE;
    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());
    for (const auto& device : devices) {
        if (SuitableDevice(device)) {
            physical_device_ = device;
            break;
        }
    }
    if (physical_device_ == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU");
    }
}

void VulkanContext::CreateLogicalDevice() {
    indices = FindQueueFamilies(physical_device_);
    if (!indices.has_value()) {
        throw std::runtime_error("not suitable queue families");
    }
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family.value(),
                                                indices.present_family.value(),
                                                indices.encode_family.value()};

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;
        queue_create_infos.push_back(queue_create_info);
    }
    VkPhysicalDeviceFeatures device_features{};
    device_features.samplerAnisotropy = VK_TRUE;
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.queueCreateInfoCount =
        static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount =
        static_cast<uint32_t>(device_extensions_.size());
    create_info.ppEnabledExtensionNames = device_extensions_.data();
    VkResult res =
        vkCreateDevice(physical_device_, &create_info, nullptr, &device_);
    VK_RES_CHECK(res);

    vkGetDeviceQueue(device_, indices.graphics_family.value(), 0,
                     &graphics_queue_);
    if (indices.present_family.has_value()) {
        vkGetDeviceQueue(device_, indices.present_family.value(), 0,
                         &present_queue_);
    }
    vkGetDeviceQueue(device_, indices.encode_family.value(), 0, &encode_queue_);


    load_VK_EXTENSIONS(instance_,vkGetInstanceProcAddr,device_,vkGetDeviceProcAddr);
}

bool VulkanContext::ValidationLayerCheck() {
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> avaliable_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, avaliable_layers.data());
    // todo: 创建一个VulkanOptions,通过参数来对比验证层结果

    return true;
}

bool VulkanContext::SuitableDevice(VkPhysicalDevice Device) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(Device, &device_properties);
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceFeatures(Device, &device_features);
    // return device_properties.deviceType ==
    // VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU&&device_features.geometryShader;
    auto family = FindQueueFamilies(Device);
    bool extension_supported = CheckDeviceExtensionSupport(Device);
    bool swap_chain_adequate = false;

    if (extension_supported) {
        VulkanSwapChainSupportDetails swap_support =
            QuerySwapChainSupport(Device);
        swap_chain_adequate = !swap_support.formats.empty() &&
                              !swap_support.present_modes.empty();
    }
    return family.has_value() && extension_supported && swap_chain_adequate;
}

bool VulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice Device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &extension_count,
                                         nullptr);
    std::vector<VkExtensionProperties> avaible_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(Device, nullptr, &extension_count,
                                         avaible_extensions.data());
    std::set<std::string> required_extensions(device_extensions_.begin(),
                                              device_extensions_.end());
    for (const auto& extension : avaible_extensions) {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

VkCtx VulkanContext::GetContext() {
    CreatePhysicalDevice();
    CreateLogicalDevice();
    // CreatePresentImage();
    VkCtx ctx;
    ctx.instance = instance_;
    ctx.physice_device = physical_device_;
    ctx.device = device_;
    ctx.queue = graphics_queue_;
    ctx.graphics_queue_idx = indices.graphics_family.value();
    return ctx;
}

void* VulkanContext::GetDeviceProcAddr(char* name) {
    return vkGetDeviceProcAddr(device_, name);
}

void* VulkanContext::GetInstanceProcAddr(char* name) {
    return vkGetInstanceProcAddr(instance_, name);
}

VulkanSwapChainSupportDetails VulkanContext::QuerySwapChainSupport(
    VkPhysicalDevice Device) {
    VulkanSwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, surface_,
                                              &details.capabilities);
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, surface_, &format_count,
                                         nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Device, surface_, &format_count,
                                             details.formats.data());
    }
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, surface_,
                                              &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Device, surface_,
                                                  &present_mode_count,
                                                  details.present_modes.data());
    }

    return details;
}

void VulkanContext::SetupDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& create_info_ext) {
    create_info_ext = {};
    create_info_ext.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info_ext.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info_ext.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info_ext.pfnUserCallback = vulkan_debug_callback;
    create_info_ext.pUserData = nullptr;
}
uint32_t VulkanContext::FindMemoryType(uint32_t TypeFilter,
                                       VkMemoryPropertyFlags Properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &mem_properties);
    for (size_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((TypeFilter & (1 << i)) &&
            ((mem_properties.memoryTypes[i].propertyFlags & Properties) ==
             Properties))
            return i;
    }
    throw std::runtime_error("failed to find suitable memory type");
}

}  // namespace toystation
