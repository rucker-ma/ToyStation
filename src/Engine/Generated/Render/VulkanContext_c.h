#pragma once
#include "Base/Macro.h"
#include "Render\VulkanContext.h"
BEGIN_EXPORT
TS_API void VulkanContext_CreateInstance(TSEngine::DebugFunction func,
                                         bool EnableDebug);
TS_API void VulkanContext_CreateSurface(HWND hwnd);
TS_API TSEngine::VkCtx VulkanContext_GetContext();
TS_API void* VulkanContext_GetDeviceProcAddr(char* name);
TS_API void* VulkanContext_GetInstanceProcAddr(char* name);
END_EXPORT
