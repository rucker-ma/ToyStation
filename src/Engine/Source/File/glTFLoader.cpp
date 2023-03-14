#define TINYGLTF_IMPLEMENTATION

#include "glTFLoader.h"

namespace toystation {
void ModelLoader::Test() {
    using namespace tinygltf;
    Model model;
    TinyGLTF loader;
    std::string err;
    std::string warn;
    loader.LoadASCIIFromFile(&model, &err, &warn, "");
}
}  // namespace toystation