#include "VulkanContext_c.h"
void VulkanContext_CreateInstance(TSEngine::DebugFunction func, bool EnableDebug)
{
    return TSEngine::VulkanContext::Instance().CreateInstance(func, EnableDebug);
}
void VulkanContext_CreateSurface(HWND hwnd)
{
    return TSEngine::VulkanContext::Instance().CreateSurface(hwnd);
}
TSEngine::VkCtx VulkanContext_GetContext()
{
    return TSEngine::VulkanContext::Instance().GetContext();
}
void * VulkanContext_GetDeviceProcAddr(char * name)
{
    return TSEngine::VulkanContext::Instance().GetDeviceProcAddr(name);
}
void * VulkanContext_GetInstanceProcAddr(char * name)
{
    return TSEngine::VulkanContext::Instance().GetInstanceProcAddr(name);
}
