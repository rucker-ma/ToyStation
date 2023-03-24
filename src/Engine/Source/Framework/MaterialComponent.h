//
// Created by ma on 2023/3/18.
//

#pragma once
#include <memory>
#include "TComponent.h"
#include "Vulkan/ResourceAllocator.h"
#include "Render/RenderContext.h"

namespace toystation{

enum TextureType{
    Texture_Normal,
    Texture_Occlusion,
    Texture_Emissive,
    Texture_Metallic_Roughness,
    Texture_Basecolor
};
using ImageType = RenderFrameType;

struct Texture{
    std::vector<unsigned char> data;
    unsigned int width;
    unsigned int height;
    std::string url;
    ImageType type;
};
class Material{
public:

    void SetTexture( TextureType type,std::shared_ptr<Texture> data);

private:
    std::shared_ptr<Texture> normal_texture_;
    std::shared_ptr<Texture> occlusion_texture_;
    std::shared_ptr<Texture> emissive_texture_;
    std::shared_ptr<Texture> metallic_roughness_;
    std::shared_ptr<Texture> basecolor_;

};

class MaterialComponent:public TComponent{
public:
    const static ComponentType Type = ComponentType::Component_Material;
    ComponentType GetType()override;
    std::shared_ptr<Material> CreateMaterial();
private:
    std::vector<std::shared_ptr<Material>> materials_;
};
}
