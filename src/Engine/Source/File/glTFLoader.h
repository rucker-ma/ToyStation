#pragma once

#include <tiny_gltf.h>
#include "Base/Vector.h"
#include "Framework/TObject.h"
#include "Framework/Component/MeshComponent.h"

namespace toystation{

class GltfModelLoader{
public:
    void Load(std::string path,std::shared_ptr<TObject>obj);
private:
    VertexDataInfo GetAccessorData(tinygltf::Model& model,int accessor_index);
    std::shared_ptr<Texture> GetTexture(tinygltf::Model& model,int texture_index);
    void SearchScene(tinygltf::Model& model, std::vector<std::shared_ptr<SubMesh>>& meshes);
    void SearchNode(tinygltf::Model& model,std::vector<std::shared_ptr<SubMesh>>& meshes,Matrix4 local_matrix,int index);
};

}