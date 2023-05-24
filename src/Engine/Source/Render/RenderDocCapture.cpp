//
// Created by ma on 2023/3/10.
//
#include "RenderDocCapture.h"

#include <renderdoc_app.h>
#ifdef _WIN32
#include <Windows.h>
#endif
#include <cassert>

#include "RenderSystem.h"
#include "ToyEngineSetting.h"

namespace toystation {

RenderDocCapture& RenderDocCapture::Instance() {
    static RenderDocCapture capture;
    return capture;
}
bool RenderDocCapture::Init() {
    if(!ToyEngineSetting::Instance().EnableRenderdoc()){
        return false;
    }
    HMODULE mod = GetModuleHandleA("renderdoc.dll");
    if (mod == NULL) {
        // TODO:通过查询注册表获取renderdoc的安装路径
        LoadLibraryA("D:\\software\\RenderDoc\\renderdoc.dll");
        mod = GetModuleHandleA("renderdoc.dll");
    }
    if (mod) {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret =
            RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&rdoc_api_);
        assert(ret == 1);
        auto rdoc_api = static_cast<RENDERDOC_API_1_5_0*>(rdoc_api_);
        rdoc_api->SetCaptureFilePathTemplate("RenderDocCapture");
        // 防止renderdoc抑制vulkan validation layer输出
        // 这个地方有个问题，如果开启此选项，vkexternalmemory会报错，实际不加载renderdoc
        // 直接使用validation layer并不会报错
        ret = rdoc_api->SetCaptureOptionU32(eRENDERDOC_Option_DebugOutputMute, 0);
        assert(ret == 1);
        return true;
    }
    return false;
}
void RenderDocCapture::StartCapture() {
    auto rdoc_api = static_cast<RENDERDOC_API_1_5_0*>(rdoc_api_);
    if (rdoc_api) {
        VkInstance instance =
            RenderSystem::kRenderGlobalData.render_context->GetContext()
                ->GetInstance();
        rdoc_api->StartFrameCapture(
            RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), NULL);
    }
}
void RenderDocCapture::EndCapture() {
    auto rdoc_api = static_cast<RENDERDOC_API_1_5_0*>(rdoc_api_);
    if (rdoc_api) {
        VkInstance instance =
            RenderSystem::kRenderGlobalData.render_context->GetContext()
                ->GetInstance();
        int res = rdoc_api->EndFrameCapture(
            RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), NULL);
        if (res != 1) {
            LogError("RenderDoc Capture Current Frame Error");
        } else {
            LogInfo("RenderDoc Capture Frame Success");
            if (rdoc_api->IsTargetControlConnected() == 0) {
                rdoc_api->LaunchReplayUI(1, NULL);
            }
            rdoc_api->ShowReplayUI();
        }
    }
}
}  // namespace toystation