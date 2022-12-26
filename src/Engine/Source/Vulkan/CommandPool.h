#pragma once
#include "Helper.h"
namespace TSEngine {
class CommandPool {
public:
    CommandPool() = default;
    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;
    ~CommandPool();

    CommandPool(
        VkDevice device, uint32_t family_index,
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        VkQueue default_queue = nullptr);
    void Init(
        VkDevice device, uint32_t family_index,
        VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        VkQueue default_queue = nullptr);
    void DeInit();
    VkCommandBuffer CreateCommandBuffer(
        VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        bool begin = true,
        VkCommandBufferUsageFlags flags =
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        const VkCommandBufferInheritanceInfo* inheritance_info = nullptr);
    void Destroy(size_t count, const VkCommandBuffer* cmds);
    void Destroy(const std::vector<VkCommandBuffer>& cmds);
    void Destroy(VkCommandBuffer cmd);
    VkCommandPool GetCommandPool() const;
    void Submit(size_t count, const VkCommandBuffer* cmds, VkQueue queue,
                VkFence fence = nullptr);
    void Submit(size_t count, const VkCommandBuffer* cmds,
                VkFence fence = nullptr);
    void Submit(const std::vector<VkCommandBuffer>& cmds,
                VkFence fence = nullptr);

    void SubmitAndWait(size_t count, const VkCommandBuffer* cmds,
                       VkQueue queue);
    void SubmitAndWait(const std::vector<VkCommandBuffer>& cmds, VkQueue queue);
    void SubmitAndWait(VkCommandBuffer cmds, VkQueue queue);
    void SubmitAndWait(VkCommandBuffer cmds);
    void SubmitAndWait(const std::vector<VkCommandBuffer>& cmds);
    void SubmitAndWait(size_t count, const VkCommandBuffer* cmds);

private:
    VkCommandPool command_pool_ = nullptr;
    VkQueue queue_ = nullptr;
    VkDevice device_ = nullptr;
};
}  // namespace TSEngine