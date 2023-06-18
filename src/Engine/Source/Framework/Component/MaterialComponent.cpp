//
// Created by ma on 2023/3/18.
//
#include "MaterialComponent.h"

#include "Render/RenderSystem.h"
namespace toystation {
void Material::SetTexture(TextureType type, std::shared_ptr<Texture> data) {
    switch (type) {
        case TextureType::Texture_Normal:
            normal_texture_ = data;
            break;
        case TextureType::Texture_Occlusion:
            occlusion_texture_ = data;
            break;
        case TextureType::Texture_Emissive:
            emissive_texture_ = data;
            break;
        case TextureType::Texture_Metallic_Roughness:
            metallic_roughness_ = data;
            break;
        case TextureType::Texture_Basecolor:
            basecolor_ = data;
            break;
    }
}
void Material::SetFactor(Vector4 base_color,float metallic,float roughness){
    factor_ = {base_color,metallic,roughness,factor_.has_normal_map};
}
ComponentType MaterialComponent::GetType() { return Component_Material; }
std::shared_ptr<Material> MaterialComponent::CreateMaterial() {
    auto material = std::make_shared<Material>();
    materials_.push_back(material);
    return material;
}

}  // namespace toystation