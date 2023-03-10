#include "TextPass.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Base/Logger.h"

namespace toystation {
void TextPass::Initialize(RenderPassInitInfo& info) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        LogError("Could not init FreeType Library");
        return;
    }
    FT_Face face;
    if(FT_New_Face(ft,"",0,&face)){
        LogError("Failed to Load font");
        return;
    }
    FT_Set_Pixel_Sizes(face,0,48);
    if(FT_Load_Char(face,'X',FT_LOAD_RENDER)){
        LogError("Failed to load Glyph");
        return;
    }
    
}
void TextPass::Draw() {}
}  // namespace toystation