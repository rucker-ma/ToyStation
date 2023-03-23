#pragma once

#include "Vulkan/DescriptorSets.h"
#include "Vulkan/ResourceAllocator.h"
#include "RenderObject.h"

namespace toystation {
class RenderResource {
public:
    struct MainCameraPassResource {
        DescriptorSetContainer set_container;
        std::vector<VkFramebuffer> framebuffers;
        RHIImage color_image;
        Texture sampler_tex;
    };
    std::unordered_map<int,std::shared_ptr<RenderObject>> render_objects_;
    MainCameraPassResource main_pass_resource_;
    VkRenderPass current_pass_;
};
}  // namespace toystation