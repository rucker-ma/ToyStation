#include "Vertex.h"
#include <vector>
namespace TSEngine {

    const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}} };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4 };


    VkVertexInputBindingDescription Vertex::GetBindingDescription()
    {
        VkVertexInputBindingDescription binding_description{};
        binding_description.binding = 0;
        binding_description.stride = sizeof(Vertex);
        binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding_description;
    }

    std::array<VkVertexInputAttributeDescription, 3> Vertex::GetAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attribute_desc{};
        attribute_desc[0].binding = 0;
        attribute_desc[0].location = 0;
        attribute_desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[0].offset = offsetof(Vertex, pos);

        attribute_desc[1].binding = 0;
        attribute_desc[1].location = 1;
        attribute_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc[1].offset = offsetof(Vertex, color);

        attribute_desc[2].binding = 0;
        attribute_desc[2].location = 2;
        attribute_desc[2].format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc[2].offset = offsetof(Vertex, texCoord);
        return attribute_desc;
    }
}
