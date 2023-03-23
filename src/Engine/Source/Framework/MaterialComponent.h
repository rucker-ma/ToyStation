//
// Created by ma on 2023/3/18.
//

#pragma once
#include <memory>
#include "TComponent.h"
#include "Vulkan/ResourceAllocator.h"

namespace toystation{

enum TextureType{
    Texture_Normal,
    Texture_Occlusion,
    Texture_Emissive,
    Texture_Metallic_Roughness,
    Texture_Basecolor
};

class Material{
public:
    void SetTexture( TextureType type,std::vector<unsigned char>data);

private:
    std::vector<unsigned char> normal_texture_;
    std::vector<unsigned char> occlusion_texture_;
    std::vector<unsigned char> emissive_texture_;
    std::vector<unsigned char> metallic_roughness_;
    std::vector<unsigned char> basecolor_;

};

class MaterialComponent:public TComponent{
public:
    ComponentType GetType()override;
    std::shared_ptr<Material> CreateMaterial();
private:
    std::vector<std::shared_ptr<Material>> materials_;
};
}
