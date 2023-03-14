#pragma once

#include "Vulkan/DescriptorSets.h"
#include "Vulkan/ResourceAllocator.h"

namespace toystation {
class RenderResource {
public:
    struct MainCameraPassResource {
        DescriptorSetContainer set_container;
        std::vector<VkFramebuffer> framebuffers;
        RHIImage color_image;
        Texture sampler_tex;
    };
//    struct ConvertPassResource{
//        DescriptorSetContainer set_container;
//        RHIImage comp_y;
//        Texture tex_y;
//        RHIImage comp_cb;
//        Texture tex_cb;
//        RHIImage comp_cr;
//        Texture tex_cr;
//        std::vector<VkFramebuffer> framebuffers;
//    };
    MainCameraPassResource main_pass_resource_;
//    ConvertPassResource convert_pass_resource_;
    VkRenderPass current_pass_;
};
}  // namespace toystation