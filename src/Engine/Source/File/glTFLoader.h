#pragma once

#include <tiny_gltf.h>
#include "Framework/TObject.h"

namespace toystation{

class GltfModelLoader{
public:
    void Load(std::string path,std::shared_ptr<TObject>obj);
private:
    std::vector<unsigned char> GetAccessorData(tinygltf::Model model,int accessor_index);
    std::vector<unsigned char>& GetTextureData(tinygltf::Model model,int texture_index);
};

}