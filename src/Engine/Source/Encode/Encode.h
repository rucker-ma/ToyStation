#pragma once


#include "Render/VulkanContext.h"

class TS_CPP_API EncodeContext {
public:
    void Init();
    void EncodeFrame();
    void Submit();

};