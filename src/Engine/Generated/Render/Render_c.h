#pragma once
#include "Base/Macro.h"
#include "Render\Render.h"
BEGIN_EXPORT
TS_API TSEngine::ImageInfo Render_GetNextImage(TSEngine::Render* Self, VkExtent2D Size);
TS_API void Render_SetScale(TSEngine::Render* Self, double Scale);
END_EXPORT
