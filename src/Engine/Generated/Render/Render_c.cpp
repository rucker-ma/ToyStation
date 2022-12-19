#include "Render_c.h"
TSEngine::ImageInfo Render_GetNextImage(TSEngine::Render* Self, VkExtent2D Size)
{
    return Self->GetNextImage(Size);
}
void Render_SetScale(TSEngine::Render* Self, double Scale)
{
    return Self->SetScale(Scale);
}
