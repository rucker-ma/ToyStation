#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Base/Vector.h"
#include "RenderPassBase.h"

namespace toystation {

class TextPass : public RenderPassBase {
public:
    virtual ~TextPass();
    virtual void Initialize(RenderPassInitInfo& info) override;
    virtual void Draw() override;
    void PostInitialize()override;
    void ResetPass()override;
private:
    struct CharCoord{
        Vector2 leftup;
        Vector2 rightup;
        Vector2 leftdown;
        Vector2 rightdown;
    };
    //保存字符的大小信息和纹理坐标
    struct Character{
        IVector2 size;
        IVector2 bearing;
        IVector2 advance;
        CharCoord coords;
    };

    enum SubPass{
        SubPass_Default=0,
        SubPass_Count
    };
    struct CharVertex{
        Vector3 position; //TODO:z分量是没有必要的，考虑取消
        Vector2 texcoord;
    };

    void PrepareRenderString(IVector2 position,std::string content);
    void LoadChar(unsigned long char_code);
    void SetupRenderPass(RenderPassInitInfo& info);
    void SetupDescriptorSetLayout(RenderPassInitInfo& info);
    void SetupPipeline(RenderPassInitInfo& info);
    void SetupFrameBuffer(RenderPassInitInfo& info);

    void AddChar(unsigned long char_code,Vector2 position,Character info);
    void UpdateUniform();
private:
    DescriptorSetContainer set_container_;
    RHITexture glyph_texture_;
    RHIBuffer vertex_buffer_;
    std::vector<CharVertex> current_render_char_;

    std::shared_ptr<RenderContext> context_;
    std::shared_ptr<RenderResource> resource_;
    std::map<unsigned long,Character> chars_;
    FT_Library ft_;
    FT_Face  face_;

    std::map<unsigned long,Character> character_buffer_;
    IVector2 tex_origin_;
    IVector2 tex_size_;
    int next_y_;

};
}  // namespace toystation