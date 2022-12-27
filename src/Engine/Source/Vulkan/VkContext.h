#pragma once
#include "Render/VulkanContext.h"

namespace toystation {

struct VkContextCreateInfo {
    struct Entry {
        Entry(const char* entry_name, bool is_optional = false)
            : name(entry_name), optional(is_optional) {}
        bool operator==(const VkExtensionProperties properties);
        std::string name;
        bool optional;
    };

    VkContextCreateInfo(bool use_validation = true);
    void AddInstanceExtension(const char* name, bool optional = false);
    void AddInstanceLayer(const char* name, bool optional = false);
    void AddDeviceExtension(const char* name, bool optional = false);
    void RemoveInstanceExtension(const char* name);
    void RemoveInstanceLayer(const char* name);
    void RmoveDeviceExtension(const char* name);
    void AddRequestedQueue(VkQueueFlags flags);
    std::string engine;
    std::string title;
    std::vector<Entry> instance_extensions;
    std::vector<Entry> instance_layers;
    std::vector<Entry> device_extensions;
    std::vector<VkQueueFlags> requested_queues;

    VkQueueFlags default_queue_gct = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    
};

bool operator==(const VkExtensionProperties properties,
                const VkContextCreateInfo::Entry& entry);

bool operator==(const VkLayerProperties layer,
                const VkContextCreateInfo::Entry& entry);
class VkContext {

    struct PhysicalDeviceInfo
    {
        
        VkPhysicalDeviceMemoryProperties memory_properties;
        std::vector<VkQueueFamilyProperties> queuefamily_properties;
    };

public:
    VkContext() = default;
    bool Init(VkContextCreateInfo& info);
    void DeInit();

private:
    bool InitInstance(VkContextCreateInfo& info);
    void InitDebugUtils();
    bool InitDevice(VkContextCreateInfo& info);

    static std::vector<VkExtensionProperties> GetInstanceExtensions();
    static std::vector<VkExtensionProperties> GetDeviceExtensions(
        VkPhysicalDevice physical_device);
    static std::vector<VkLayerProperties> GetInstanceLayers();

    static bool FillFilteredExtensions(
        const std::vector<VkExtensionProperties>& properties,
        const std::vector<VkContextCreateInfo::Entry>& requested,
        std::vector<std::string>& used);
    static bool FillFilteredLayers(
        const std::vector<VkLayerProperties>& properties,
        const std::vector<VkContextCreateInfo::Entry>& requested,
        std::vector<std::string>& used);

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_serverity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data);

    VkInstance instance_;
    PhysicalDeviceInfo physical_device_info_;
    VkPhysicalDevice physical_device_;
    VkDevice device_;
    VkDebugUtilsMessengerEXT debug_messenger_;

    std::vector<std::string> used_instance_layers_;
    std::vector<std::string> used_instance_extensions_;
    std::vector<std::string> used_device_extensions_;
};
}  // namespace toystation