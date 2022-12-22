#pragma once
#include "ImageFactory.h"
#include "RenderUtils.h"
#include "Vertex.h"

#define MAX_FRAMES_IN_FLIGHT 3

namespace TSEngine {

struct UniformBufferOjbect {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

CLASS()
class TS_CPP_API Render {
public:
    virtual ~Render();
    void Init();
    void Draw();
    FUNCTION(CSHARP)
    ImageInfo GetNextImage(VkExtent2D Size);
    void UpdateSize(VkRect2D Size);
    FUNCTION(CSHARP)
    void SetScale(double Scale);
    FUNCTION(CSHARP)
    void FrameResized();
private:
    void CreateSwapChain();
    void RecreateSwapChain();
    void CleanSwapChain();
    void CreateCommandPool();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateCommandBuffer();
    void CreateSyncObjects();
    void CreateFrameBuffers();
    void CreatePresentImage();
    void CreateTextures();
    void CreateUniformBuffer();
    void CreateUtils();

    VkFormat FindDepthFormat();

    void RecordCommandBuffer(VkCommandBuffer CommandBuffer,
                             uint32_t image_index);

    void UpdateUniformBuffer(uint32_t current_image);

private:
    VkRenderPass render_pass_;
    VkPipeline graphics_pipeline_;
    VkPipelineLayout pipeline_layout_;
    VkDescriptorSetLayout descriptor_set_layout_;
    VkDescriptorPool descriptor_pool_;
    VkCommandPool command_pool_;
    VKSwapChainInfo info_;

    std::vector<VkCommandBuffer> commandbuffers_;
    std::vector<VkDescriptorSet> descritpor_sets_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> inflight_fences_;
    std::vector<VkFramebuffer> swapchain_framebuffers_;
    std::vector<std::shared_ptr<Image>> swap_chain_images_;

    std::shared_ptr<Image> texture_;
    std::shared_ptr<Image> depth_texture_;
    GPUBuffer<Vertex>* vertex_buffer_;
    GPUBuffer<uint16_t>* indices_buffer_;
    std::vector<GPUBuffer<UniformBufferOjbect>*> uniform_buffers_;
    uint32_t current_frame_;
    RenderUtils* utils_;
    std::shared_ptr<Image> present_image;
    // draw content size
    VkRect2D copy_size_;
    double scale_;
    bool frame_resized_;
};
}  // namespace TSEngine
