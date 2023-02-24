#include "VkContext.h"

#include "Helper.h"
namespace toystation {

void AddExtension(std::vector<VkContextCreateInfo::Entry>& container,
                  const char* name, bool optional) {
    VkContextCreateInfo::Entry entry(name, optional);
    container.push_back(entry);
}

VkContextCreateInfo::VkContextCreateInfo(bool use_validation) {
#ifdef _DEBUG
    AddExtension(instance_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true);
    if (use_validation) {
        AddExtension(instance_layers, "VK_LAYER_KHRONOS_validation", true);
    }
#endif
    if (default_queue_gct) {
        requested_queues.push_back(default_queue_gct);
    }
}
void VkContextCreateInfo::AddInstanceExtension(const char* name,
                                               bool optional) {
    AddExtension(instance_extensions, name, optional);
}
void VkContextCreateInfo::AddInstanceLayer(const char* name, bool optional) {
    AddExtension(instance_layers, name, optional);
}
void VkContextCreateInfo::AddDeviceExtension(const char* name, bool optional) {
    AddExtension(device_extensions, name, optional);
}
void VkContextCreateInfo::RemoveInstanceExtension(const char* name) {
    for (auto extension = instance_extensions.begin();
         extension != instance_extensions.end(); extension++) {
        if ((*extension).name == std::string(name)) {
            instance_extensions.erase(extension);
            break;
        }
    }
}
void VkContextCreateInfo::RemoveInstanceLayer(const char* name) {
    for (auto layer = instance_layers.begin(); layer != instance_layers.end();
         layer++) {
        if ((*layer).name == std::string(name)) {
            instance_layers.erase(layer);
            break;
        }
    }
}
void VkContextCreateInfo::RmoveDeviceExtension(const char* name) {
    for (auto extension = device_extensions.begin();
         extension != device_extensions.end(); extension++) {
        if ((*extension).name == std::string(name)) {
            device_extensions.erase(extension);
            break;
        }
    }
}
void VkContextCreateInfo::AddRequestedQueue(VkQueueFlags flags) {
    requested_queues.push_back(flags);
}
bool VkContext::Init(VkContextCreateInfo& info) { return InitInstance(info); }
void VkContext::DeInit() {}
void VkContext::CreateRenderPass(VkRenderPassCreateInfo& create_info,
                                 VkRenderPass& render_pass) {
    vkCreateRenderPass(device_, &create_info, nullptr, &render_pass);
}
void VkContext::CreateDescriptorSetLayout(
    VkDescriptorSetLayoutCreateInfo& create_info,
    VkDescriptorSetLayout& layout) {
    vkCreateDescriptorSetLayout(device_, &create_info, nullptr, &layout);
}
void VkContext::CreatePipelineLayout(VkPipelineLayoutCreateInfo& create_info,
                                     VkPipelineLayout& layout) {
    vkCreatePipelineLayout(device_, &create_info, nullptr, &layout);
}
void VkContext::CreateGraphicsPipeline(
    uint32_t create_info_count, const VkGraphicsPipelineCreateInfo* create_info,
    VkPipeline& pipeline) {
    vkCreateGraphicsPipelines(device_, nullptr, create_info_count, create_info,
                              nullptr, &pipeline);
}

void VkContext::CreateComputePipeline(
    uint32_t create_info_count, const VkComputePipelineCreateInfo* create_info,
    VkPipeline& pipeline) {
    vkCreateComputePipelines(device_, nullptr, create_info_count, create_info,
                             nullptr, &pipeline);
}

void VkContext::CreateDescriptorPool(uint32_t pool_size_count,
                                     const VkDescriptorPoolSize* pool_size,
                                     uint32_t max_sets,
                                     VkDescriptorPool& pool) {
    VkDescriptorPoolCreateInfo create_info;
    ZeroVKStruct(create_info, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
    create_info.pNext = nullptr;
    create_info.maxSets = max_sets;
    create_info.poolSizeCount = pool_size_count;
    create_info.pPoolSizes = pool_size;
    vkCreateDescriptorPool(device_, &create_info, nullptr, &pool);
}

void VkContext::CreateFramebuffer(const VkFramebufferCreateInfo& info,
                                  VkFramebuffer& buffer) {
    vkCreateFramebuffer(device_, &info, nullptr, &buffer);
}

VkShaderModule VkContext::CreateShader(const char* data, size_t size) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = size;
    create_info.pCode = reinterpret_cast<const uint32_t*>(data);
    VkShaderModule shader_module = nullptr;
    vkCreateShaderModule(device_, &create_info, nullptr, &shader_module);
    return shader_module;
}

VkQueue VkContext::GetQueue(VkQueueFlags flags) { return graphics_queue_; }

uint32_t VkContext::GetQueueFamilyIndex(VkQueueFlags flags) {
    return queue_family_;
}

bool VkContext::InitInstance(VkContextCreateInfo& info) {
    // create instance
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "toystation";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_MAKE_VERSION(1, 2, 0);
    LogDebug("Load Vulkan Instance layers");
    auto layer_properties = GetInstanceLayers();

    for (auto&& layer : layer_properties) {
        LogDebug("Supported Layer: " + layer.layerName);
    }

    if (!FillFilteredLayers(layer_properties, info.instance_layers,
                            used_instance_layers_)) {
        LogError("FillFilteredLayers Error");
        return false;
    }
    LogDebug("Load Vulkan Instance Extensions");
    auto extensions = GetInstanceExtensions();

    for (auto&& exten : extensions) {
        LogDebug("Supported instance extensions: " + exten.extensionName);
    }

    if (!FillFilteredExtensions(extensions, info.instance_extensions,
                                used_instance_extensions_)) {
        LogError("FillFilteredExtensions Error");
        return false;
    }
    std::vector<const char*> used_extension;
    std::vector<const char*> used_layers;
    for (const auto& ext : used_instance_extensions_) {
        used_extension.push_back(ext.c_str());
    }
    for (const auto& layer : used_instance_layers_) {
        used_layers.push_back(layer.c_str());
    }

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = used_extension.size();
    create_info.ppEnabledExtensionNames = used_extension.data();
    create_info.enabledLayerCount = used_layers.size();
    create_info.ppEnabledLayerNames = used_layers.data();
    create_info.pNext = nullptr;
    vkCreateInstance(&create_info, nullptr, &instance_);
    // check enable debug

    if (std::find_if(
            used_extension.begin(), used_extension.end(), [](const char* data) {
                return strcmp(data, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
            }) != used_extension.end()) {
        InitDebugUtils();
    }
    InitDevice(info);
    return true;
}
void VkContext::InitDebugUtils() {
    LogInfo("Set Vulkan Debug layer");
    auto create_debug_messenger_ext =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance_, "vkCreateDebugUtilsMessengerEXT");
    // auto destroy_debug_messenger_ext =
    //     (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
    //         instance_, "vkDestroyDebugUtilsMessengerEXT");

    if (create_debug_messenger_ext != nullptr) {
        VkDebugUtilsMessengerCreateInfoEXT create_info_ext;
        ZeroVKStruct(create_info_ext,
                     VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
        create_info_ext.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info_ext.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info_ext.pfnUserCallback = VulkanDebugCallback;
        create_info_ext.pUserData = this;
        create_debug_messenger_ext(instance_, &create_info_ext, nullptr,
                                   &debug_messenger_);
    }
}

bool VkContext::InitDevice(VkContextCreateInfo& info) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance_, &count, nullptr);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance_, &count, devices.data());
    for (auto&& device : devices) {
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        std::vector<VkExtensionProperties> avalible_extensions(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count,
                                             avalible_extensions.data());
        used_device_extensions_.clear();
        if (FillFilteredExtensions(avalible_extensions, info.device_extensions,
                                   used_device_extensions_)) {
            physical_device_ = device;
            break;
        }
    }
    vkGetPhysicalDeviceMemoryProperties(
        physical_device_, &physical_device_info_.memory_properties);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &count, nullptr);
    physical_device_info_.queuefamily_properties.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physical_device_, &count,
        physical_device_info_.queuefamily_properties.data());

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    float queue_priority = 1.0;
    for (auto&& request_queue : info.requested_queues) {
        for (uint32_t idx = 0;
             idx < physical_device_info_.queuefamily_properties.size(); idx++) {
            // 通过位运算找到支持的queue family
            if ((request_queue &
                 physical_device_info_.queuefamily_properties[idx]
                     .queueFlags) == request_queue) {
                VkDeviceQueueCreateInfo queue_create_info;
                ZeroVKStruct(queue_create_info,
                             VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
                queue_create_info.queueFamilyIndex = idx;
                queue_create_info.queueCount = 1;
                queue_create_info.pQueuePriorities = &queue_priority;
                queue_create_infos.push_back(queue_create_info);
                queue_family_ = idx;
                break;
            }
        }
    }

    VkDeviceCreateInfo device_create_info;
    ZeroVKStruct(device_create_info, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
    device_create_info.queueCreateInfoCount = queue_create_infos.size();
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    std::vector<const char*> used_extensions;
    for (auto&& ext : used_device_extensions_) {
        used_extensions.push_back(ext.data());
    }
    device_create_info.enabledExtensionCount = used_extensions.size();
    device_create_info.ppEnabledExtensionNames = used_extensions.data();
    vkCreateDevice(physical_device_, &device_create_info, nullptr, &device_);
    load_VK_EXTENSIONS(instance_, vkGetInstanceProcAddr, device_,
                       vkGetDeviceProcAddr);
    // TODO: get queue
    vkGetDeviceQueue(device_, queue_create_infos[0].queueFamilyIndex, 0,
                     &graphics_queue_);

    return true;
}

std::vector<VkExtensionProperties> VkContext::GetInstanceExtensions() {
    uint32_t count = 0;
    std::vector<VkExtensionProperties> extensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    extensions.resize(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    return extensions;
}
std::vector<VkExtensionProperties> VkContext::GetDeviceExtensions(
    VkPhysicalDevice physical_device) {
    uint32_t count = 0;
    std::vector<VkExtensionProperties> extensions;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count,
                                         nullptr);
    extensions.resize(count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count,
                                         extensions.data());
    return extensions;
}
std::vector<VkLayerProperties> VkContext::GetInstanceLayers() {
    uint32_t count = 0;
    std::vector<VkLayerProperties> layers;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    layers.resize(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    return layers;
}
bool VkContext::FillFilteredExtensions(
    const std::vector<VkExtensionProperties>& properties,
    const std::vector<VkContextCreateInfo::Entry>& requested,
    std::vector<std::string>& used) {
    for (auto&& entry : requested) {
        auto find_result =
            std::find(properties.begin(), properties.end(), entry);
        if (find_result != properties.end()) {
            used.push_back(entry.name);
        } else if (!entry.optional) {
            LogError("Cannot found request property: " + entry.name);
            return false;
        } else {
            LogWarn("Not found request property: " + entry.name);
        }
    }
    return true;
}
bool VkContext::FillFilteredLayers(
    const std::vector<VkLayerProperties>& properties,
    const std::vector<VkContextCreateInfo::Entry>& requested,
    std::vector<std::string>& used) {
    for (auto&& entry : requested) {
        auto find_result =
            std::find(properties.begin(), properties.end(), entry);
        if (find_result != properties.end()) {
            used.push_back(entry.name);
        } else if (!entry.optional) {
            LogError("Cannot found request property: " + entry.name);
            return false;
        } else {
            LogWarn("Not found request property: " + entry.name);
        }
    }
    return true;
}
VKAPI_ATTR VkBool32 VKAPI_CALL VkContext::VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_serverity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    switch (message_serverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            LogDebug(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            LogInfo(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            LogWarn(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            LogError(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
        default:
            break;
    }

    return 0;
}
bool operator==(const VkExtensionProperties properties,
                const VkContextCreateInfo::Entry& entry) {
    return std::string(static_cast<const char*>(properties.extensionName)) ==
           entry.name;
}
bool operator==(const VkLayerProperties layer,
                const VkContextCreateInfo::Entry& entry) {
    return std::string(static_cast<const char*>(layer.layerName)) == entry.name;
}
}  // namespace toystation