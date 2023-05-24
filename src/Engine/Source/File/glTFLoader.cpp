#define TINYGLTF_IMPLEMENTATION

#include "glTFLoader.h"

#include "Base/Logger.h"
#include "FileUtil.h"

namespace toystation {

namespace {
Matrix4 ToMatrix4(std::vector<double> data){
    assert(data.size()==16&&"vector to matrix size must equal 16");
    return Matrix4 (data[0],data[1],data[2],data[3],
                   data[4],data[5],data[6],data[7],
                   data[8],data[9],data[10],data[11],
                   data[12],data[13],data[14],data[15]);
}
Vector4 ToVector4(std::vector<double> data){
    assert(data.size()==4&&"vector to vector4 size must equal 4");
    return Vector4 (data[0],data[1],data[2],data[3]);
}

}

constexpr char kMaterailSpecularExtensionName[] =
    "KHR_materials_pbrSpecularGlossiness";
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
    if (!model.materials.empty()) {
        std::shared_ptr<MaterialComponent> material_comp =
            obj->CreateComponent<MaterialComponent>();
        for (auto& gltf_material : model.materials) {
            std::shared_ptr<Material> material =
                material_comp->CreateMaterial();
            // normal texture
            if (gltf_material.normalTexture.index >= 0) {
                auto data =
                    GetTexture(model, gltf_material.normalTexture.index);
                material->SetTexture(TextureType::Texture_Normal, data);
                material->Factor().has_normal_map = 1;
            }else{
                material->Factor().has_normal_map = 0;
            }
            // basecolor texture, basecolor factor?
            if (gltf_material.pbrMetallicRoughness.baseColorTexture.index >=
                0) {
                auto data = GetTexture(
                    model,
                    gltf_material.pbrMetallicRoughness.baseColorTexture.index);
                material->SetTexture(TextureType::Texture_Basecolor, data);
            }
            // metallic roughness texture, factor?
            if (gltf_material.pbrMetallicRoughness.metallicRoughnessTexture
                    .index >= 0) {
                auto data =
                    GetTexture(model, gltf_material.pbrMetallicRoughness
                                          .metallicRoughnessTexture.index);
                material->SetTexture(TextureType::Texture_Metallic_Roughness,
                                     data);

                material->SetFactor(ToVector4(gltf_material.pbrMetallicRoughness.baseColorFactor),
                                    gltf_material.pbrMetallicRoughness.metallicFactor,
                                    gltf_material.pbrMetallicRoughness.roughnessFactor);
            }
            // occlusion texture
            if (gltf_material.occlusionTexture.index >= 0) {
                auto data =
                    GetTexture(model, gltf_material.occlusionTexture.index);
                material->SetTexture(TextureType::Texture_Occlusion, data);
            }
            // emissive texture,but emissive factor ?
            if (gltf_material.emissiveTexture.index >= 0) {
                auto data =
                    GetTexture(model, gltf_material.emissiveTexture.index);
                material->SetTexture(TextureType::Texture_Emissive, data);
            }
            if (gltf_material.extensions.find(kMaterailSpecularExtensionName) !=
                gltf_material.extensions.end()) {
                tinygltf::Value& specular_extension =
                    gltf_material.extensions[kMaterailSpecularExtensionName];
                int index = specular_extension.Get("diffuseTexture")
                                .Get("index")
                                .GetNumberAsInt();
                if (index >= 0) {
                    auto data = GetTexture(model, index);
                    material->SetTexture(TextureType::Texture_Basecolor, data);
                }
                index = specular_extension.Get("specularGlossinessTexture")
                            .Get("index")
                            .GetNumberAsInt();
                if (index >= 0) {
                    auto data = GetTexture(model, index);
                    material->SetTexture(
                        TextureType::Texture_Metallic_Roughness, data);
                }
                float glossiness_factor =
                    specular_extension.Get("glossinessFactor")
                        .GetNumberAsDouble();
                //TODO:set factor
                material->SetPBRMode(PBRMode::PBR_Specular_Glossiness);
            } else {
                material->SetPBRMode(PBRMode::PBR_Metallic_Roughness);
            }

            material_cache.push_back(material);
        }
    }
    std::vector<std::shared_ptr<SubMesh>> meshes;
    if (!model.meshes.empty()) {
        std::shared_ptr<MeshComponent> mesh_comp =
            obj->CreateComponent<MeshComponent>();
        for (auto& mesh : model.meshes) {
//            std::shared_ptr<SubMesh> sub_mesh = mesh_comp->CreateSubMesh();
            std::shared_ptr<SubMesh> sub_mesh = std::make_shared<SubMesh>();
                meshes.push_back(sub_mesh);
            // TODO:一个mesh多个primitive怎么额解决？
            for (auto& primitive : mesh.primitives) {
                // primitive.mode 渲染模式，TINYGLTF_MODE_*
                // primitive.attributes
                // attributes中类型索引指向accessor,获取具体的数据
                // primitive.material  材质索引
                auto indices_data = GetAccessorData(model, primitive.indices);
                sub_mesh->AddData(MeshDataType::Mesh_Indices, indices_data);

                sub_mesh->SetMaterialIndex(primitive.material);
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
                        sub_mesh->AddData(MeshDataType::Mesh_Coord, attr_data);
                    } else {
                        LogError("MeshComponent unsupported Data");
                    }
                }
            }
            mesh_comp->AddSubMesh(sub_mesh);
        }
    }
    //通过scene和node获取mesh的local matrix
    SearchScene(model,meshes);
}
VertexDataInfo GltfModelLoader::GetAccessorData(tinygltf::Model& model,
                                                int accessor_index) {
    tinygltf::Accessor& accessor =
        model.accessors[accessor_index];  // 获取顶点indices数据
    tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    tinygltf::Buffer& buffer = model.buffers[view.buffer];
    // begin -end : bufferview range
    auto view_buffer_begin = buffer.data.begin() + view.byteOffset;
    auto view_buffer_end = view_buffer_begin + view.byteLength;

    int length = accessor.count *
                 tinygltf::GetComponentSizeInBytes(accessor.componentType) *
                 tinygltf::GetNumComponentsInType(accessor.type);
    // add accessor offset for accessor
    auto accessor_data_begin = view_buffer_begin + accessor.byteOffset;
    auto accessor_data_end = accessor_data_begin + length;
    assert(accessor_data_end <= view_buffer_end);
    VertexDataInfo result;
    result.buffer = std::move(
        std::vector<unsigned char>(accessor_data_begin, accessor_data_end));
    switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_SHORT:
            result.type = BufferType::TYPE_SHORT;
            break;
        case TINYGLTF_COMPONENT_TYPE_INT:
            result.type = BufferType::TYPE_INT;
            break;
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            result.type = BufferType::TYPE_FLOAT;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            result.type = BufferType::TYPE_UNSIGNED_SHORT;
            break ;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            result.type = BufferType::TYPE_UNSIGNED_INT;
            break ;
    }
    result.nums = accessor.count;
    return result;
}
std::shared_ptr<Texture> GltfModelLoader::GetTexture(tinygltf::Model& model,
                                                     int texture_index) {
    tinygltf::Texture& tex = model.textures[texture_index];
    tinygltf::Image& image = model.images[tex.source];

    auto path = image.uri;
    // other attr : pixel_type  component width height
    tinygltf::Sampler& sampler = model.samplers[tex.sampler];
    // sampler.minFilter
    // sampler.magFilter
    // sampler.wrapS
    // sampler.wrapT

    auto texture = std::make_shared<Texture>();
    texture->width = image.width;
    texture->height = image.height;
    texture->data = image.image;
    switch (image.component) {  // compoent is image channel
        case 1:
            texture->type = FRAME_RED;
            assert(0 && "type not support");
            break;
        case 2:
            texture->type = FRAME_RG;
            assert(0 && "type not support");
            break;
        case 3:
            texture->type = FRAME_RGB;
            break;
        case 4:
            texture->type = FRAME_RGBA;
            break;
        default:
            assert(0&&"not support format");
            break ;
    }
    return texture;
}
//TODO: camera not consider
void GltfModelLoader::SearchScene(tinygltf::Model& model, std::vector<std::shared_ptr<SubMesh>>& meshes) {
    for(auto&scene: model.scenes) {
        Matrix4 identity(1);
        for(auto node_index: scene.nodes){
            SearchNode(model,meshes,identity,node_index);
        }
    }
}
void GltfModelLoader::SearchNode(tinygltf::Model& model,
                                 std::vector<std::shared_ptr<SubMesh>>& meshes,
                                 Matrix4 local_matrix,int index){
    tinygltf::Node& node = model.nodes[index];
    Matrix4 current_mat = local_matrix;
    if(!node.matrix.empty()){
        current_mat*=ToMatrix4(node.matrix);
    }
    if(node.mesh>=0&&node.mesh<meshes.size()){
        meshes[node.mesh]->SetLocalMatrix(current_mat);
    }
    for(auto&child_index:node.children){
        SearchNode(model,meshes,current_mat,child_index);
    }
}
}  // namespace toystation