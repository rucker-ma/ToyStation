#define TINYGLTF_IMPLEMENTATION

#include "glTFLoader.h"

#include "Base/Logger.h"
#include "FileUtil.h"
#include "Framework/MeshComponent.h"

namespace toystation {
static const std::map<std::string, MeshDataType> kMeshTypeSpelling{
    {"NORMAL", MeshDataType::Mesh_Normal},
    {"POSITION", MeshDataType::Mesh_Position},
    {"TANGENT", MeshDataType::Mesh_Tangent}};

void GltfModelLoader::Load(std::string path, std::shared_ptr<TObject> obj) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn,
                                        FileUtil::Combine(path.c_str()));
    if (!res) {
        LogWarn("Load gltf model error: " + err);
    }
    // scene场景节点中包含node节点，node节点有不同的层级关系，包含指向不同mesh的局部变换
    //

    std::vector<std::weak_ptr<Material>> material_cache;
    if(!model.materials.empty()){
        std::shared_ptr<MaterialComponent> material_comp =
            obj->CreateComponent<MaterialComponent>();
        for(auto& gltf_material:model.materials){

             std::shared_ptr<Material> material = material_comp->CreateMaterial();
             // normal texture
             if(gltf_material.normalTexture.index >=0) {
                 auto data =
                     GetTextureData(model, gltf_material.normalTexture.index);
                 material->SetTexture(Texture_Normal, data);
             }
             //basecolor texture, basecolor factor?
             if(gltf_material.pbrMetallicRoughness.baseColorTexture.index>=0){
                 auto data =
                     GetTextureData(model, gltf_material.pbrMetallicRoughness.baseColorTexture.index);
                 material->SetTexture(Texture_Basecolor, data);
             }
             // metallic roughness texture, factor?
            if(gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index>=0){
                 auto data =
                     GetTextureData(model, gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index);
                 material->SetTexture(Texture_Metallic_Roughness, data);
            }
            // occlusion texture
            if(gltf_material.occlusionTexture.index>=0){
                 auto data =
                     GetTextureData(model, gltf_material.occlusionTexture.index);
                 material->SetTexture(Texture_Occlusion, data);
            }
            // emissive texture,but emissive factor ?
            if(gltf_material.emissiveTexture.index>=0){
                 auto data =
                     GetTextureData(model, gltf_material.emissiveTexture.index);
                 material->SetTexture(Texture_Emissive, data);
            }
            material_cache.push_back(material);
        }
    }

    if (!model.meshes.empty()) {
        std::shared_ptr<MeshComponent> mesh_comp =
            obj->CreateComponent<MeshComponent>();

        for (auto& mesh : model.meshes) {
            std::shared_ptr<SubMesh> sub_mesh = mesh_comp->CreateSubMesh();
            //TODO:一个mesh多个primitive怎么额解决？
            for (auto& primitive : mesh.primitives) {
                // primitive.mode 渲染模式，TINYGLTF_MODE_*
                // primitive.attributes
                // attributes中类型索引指向accessor,获取具体的数据
                // primitive.material  材质索引
                auto indices_data = GetAccessorData(model, primitive.indices);
                sub_mesh->AddData(MeshDataType::Mesh_Indices, indices_data);
                // get material
                // Material& material = model.materials[primitive.material];
                for (auto& pair : primitive.attributes) {
                    std::string attr_name = pair.first;
                    auto attr_data = GetAccessorData(model, pair.second);
                    if (kMeshTypeSpelling.find(attr_name) !=
                        kMeshTypeSpelling.end()) {
                        MeshDataType mesh_data_type =
                            kMeshTypeSpelling.at(attr_name);
                        sub_mesh->AddData(mesh_data_type, attr_data);
                    } else if (attr_name.find("TEXCOORD") !=
                               std::string::npos) {
                        // 添加纹理
                        sub_mesh->AddTexture(attr_data,material_cache[primitive.material]);
                    } else {
                        LogError("MeshComponent unsupported Data");
                    }
                }
            }
        }
    }
}
std::vector<unsigned char> GltfModelLoader::GetAccessorData(
    tinygltf::Model model, int accessor_index) {
    tinygltf::Accessor& accessor =
        model.accessors[accessor_index];  // 获取顶点indices数据
    tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    tinygltf::Buffer& buffer = model.buffers[view.buffer];
    auto begin =
        buffer.data.begin() + view.byteOffset + accessor.byteOffset;
    auto end = begin + view.byteLength;
    return std::vector<unsigned char>(begin, end);
}
std::vector<unsigned char>& GltfModelLoader::GetTextureData(
    tinygltf::Model model, int texture_index) {
    tinygltf::Texture& tex = model.textures[texture_index];
    tinygltf::Image& image = model.images[tex.source];

    auto path = image.uri;
    // other attr : pixel_type  component width height
    tinygltf::Sampler& sampler = model.samplers[tex.sampler];
    // sampler.minFilter
    // sampler.magFilter
    // sampler.wrapS
    // sampler.wrapT
    return image.image;
}

}  // namespace toystation