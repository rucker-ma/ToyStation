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

namespace toystation {

RenderDocCapture& RenderDocCapture::Instance() {
    static RenderDocCapture capture;
    return capture;
}
bool RenderDocCapture::Init() {
    HMODULE mod = GetModuleHandleA("renderdoc.dll");
    if(mod == NULL) {
        LoadLibraryA(
            "D:\\project\\ToyStation\\src\\Editor\\Editor\\bin\\Debug\\net6.0\\renderdoc.dll");
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
        return true;
    }
    return false;
}
void RenderDocCapture::StartCapture() {
    auto rdoc_api = static_cast<RENDERDOC_API_1_5_0*>(rdoc_api_);
    if (rdoc_api) {
        VkInstance instance = RenderSystem::kRenderGlobalData.render_context->GetContext()->GetInstance();
        rdoc_api->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), NULL);
    }
}
void RenderDocCapture::EndCapture() {
    auto rdoc_api = static_cast<RENDERDOC_API_1_5_0*>(rdoc_api_);
    if (rdoc_api) {
        VkInstance instance = RenderSystem::kRenderGlobalData.render_context->GetContext()->GetInstance();
        int res = rdoc_api->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(instance), NULL);
        if(res !=1) {
            LogError("RenderDoc Capture Current Frame Error");
        }else{
            LogInfo("RenderDoc Capture Frame Success");
//            rdoc_pid_ = rdoc_api->LaunchReplayUI(1,NULL);
        }
    }
}
}  // namespace toystation