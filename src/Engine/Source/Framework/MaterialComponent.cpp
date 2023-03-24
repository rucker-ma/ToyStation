//
// Created by ma on 2023/3/18.
//
#include "MaterialComponent.h"

#include "Render/RenderSystem.h"
namespace toystation {
void Material::SetTexture(TextureType type, std::shared_ptr<Texture> data) {
    switch (type) {
        case Texture_Normal:
            break;
        case Texture_Occlusion:
            break;
        case Texture_Emissive:
            break;
        case Texture_Metallic_Roughness:
            break;
        case Texture_Basecolor:
            break;
    }
}
ComponentType MaterialComponent::GetType() { return Component_Material; }
std::shared_ptr<Material> MaterialComponent::CreateMaterial() {
    auto material = std::make_shared<Material>();
    materials_.push_back(material);
    return material;
}

}  // namespace toystation