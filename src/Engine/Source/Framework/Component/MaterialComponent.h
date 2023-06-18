//
// Created by ma on 2023/3/18.
//

#pragma once
#include <memory>

#include "Base/Vector.h"
#include "Render/RenderContext.h"
#include "TComponent.h"
#include "Vulkan/ResourceAllocator.h"

namespace toystation{

enum class BufferType{
    TYPE_BYTE,
    TYPE_SHORT,
    TYPE_UNSIGNED_SHORT,
    TYPE_INT,
    TYPE_UNSIGNED_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE
};
struct VertexDataInfo{
    std::vector<unsigned char> buffer;
    BufferType type;
    int nums; //表示buffer对于的顶点个数
};

enum class TextureType{
    Texture_Normal,
    Texture_Occlusion,
    Texture_Emissive,
    Texture_Metallic_Roughness,
    Texture_Basecolor
};
enum class PBRMode{
    PBR_Metallic_Roughness,
    PBR_Specular_Glossiness
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
    void SetPBRMode(PBRMode mode){mode_ = mode;}
    void SetFactor(Vector4 base_color,float metallic,float roughness);
    std::shared_ptr<Texture> NormalTexture(){return normal_texture_;}
    std::shared_ptr<Texture> OcclusionTexture(){return occlusion_texture_;}
    std::shared_ptr<Texture> EmissiveTexture(){return emissive_texture_;}
    std::shared_ptr<Texture> MetallicRoughnessTexture(){return metallic_roughness_;}
    std::shared_ptr<Texture> Basecolor(){return basecolor_;}
    struct MaterialFactor{
        Vector4 base_color;
        float metallic_factor;
        float roughness_factor;
        int has_normal_map; //use int because glsl bool = 4 bytes but c++ bool = 1 byte
    };
    MaterialFactor& Factor(){return factor_;}
private:
    std::shared_ptr<Texture> normal_texture_;
    std::shared_ptr<Texture> occlusion_texture_;
    std::shared_ptr<Texture> emissive_texture_;
    std::shared_ptr<Texture> metallic_roughness_;
    std::shared_ptr<Texture> basecolor_;
    PBRMode mode_;
    MaterialFactor factor_;
};

class MaterialComponent:public TComponent{
public:
    const static ComponentType Type = ComponentType::Component_Material;
    ComponentType GetType()override;
    std::shared_ptr<Material> CreateMaterial();
    std::vector<std::shared_ptr<Material>>& Materials(){return materials_;}
private:
    std::vector<std::shared_ptr<Material>> materials_;
};
}
