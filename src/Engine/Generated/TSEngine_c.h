#pragma once
#include "Base/Macro.h"
#include "TSEngine.h"
BEGIN_EXPORT
TS_API void TEngine_Init();
TS_API void TEngine_Tick();
TS_API TSEngine::Render * TEngine_IRender();
TS_API void TEngine_UpdateSize(VkRect2D Size);
END_EXPORT
