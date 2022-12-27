#include "CommandPool.h"

namespace toystation {

CommandPool::~CommandPool() { DeInit(); }
CommandPool::CommandPool(VkDevice device, uint32_t family_index,
                         VkCommandPoolCreateFlags flags,
                         VkQueue default_queue) {
    Init(device, family_index, flags, default_queue);
}

void CommandPool::Init(VkDevice device, uint32_t family_index,
                       VkCommandPoolCreateFlags flags, VkQueue default_queue) {
    device_ = device;
    VkCommandPoolCreateInfo info;
    ZeroVKStruct(info, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    info.flags = flags;
    info.queueFamilyIndex = family_index;
    vkCreateCommandPool(device_, &info, nullptr, &command_pool_);
    if (default_queue) {
        queue_ = default_queue;
    } else {
        vkGetDeviceQueue(device_, family_index, 0, &queue_);
    }
}

void CommandPool::DeInit() {
    if (command_pool_) {
        vkDestroyCommandPool(device_, command_pool_, nullptr);
        command_pool_ = nullptr;
    }
    device_ = nullptr;
}

VkCommandBuffer CommandPool::CreateCommandBuffer(
    VkCommandBufferLevel level, bool begin, VkCommandBufferUsageFlags flags,
    const VkCommandBufferInheritanceInfo* inheritance_info) {
    VkCommandBufferAllocateInfo alloc_info;
    ZeroVKStruct(alloc_info, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    alloc_info.level = level;
    alloc_info.commandPool = command_pool_;
    alloc_info.commandBufferCount = 1;
    VkCommandBuffer cmd = nullptr;
    vkAllocateCommandBuffers(device_, &alloc_info, &cmd);
    if (begin) {
        VkCommandBufferBeginInfo begin_info;
        ZeroVKStruct(begin_info, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
        begin_info.flags = flags;
        begin_info.pInheritanceInfo = inheritance_info;
        vkBeginCommandBuffer(cmd, &begin_info);
    }

    return cmd;
}

void CommandPool::Destroy(size_t count, const VkCommandBuffer* cmds) {
    vkFreeCommandBuffers(device_, command_pool_, (uint32_t)count, cmds);
}

void CommandPool::Destroy(const std::vector<VkCommandBuffer>& cmds) {
    Destroy(cmds.size(), cmds.data());
}

void CommandPool::Destroy(VkCommandBuffer cmd) { Destroy(1, &cmd); }

VkCommandPool CommandPool::GetCommandPool() const { return command_pool_; }

void CommandPool::Submit(size_t count, const VkCommandBuffer* cmds,
                         VkQueue queue, VkFence fence) {
    for (size_t idx = 0; idx < count; idx++) {
        vkEndCommandBuffer(cmds[idx]);
    }
    VkSubmitInfo submit_info;
    ZeroVKStruct(submit_info, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submit_info.pCommandBuffers = cmds;
    submit_info.commandBufferCount = (uint32_t)count;
    vkQueueSubmit(queue, 1, &submit_info, fence);
}

void CommandPool::Submit(size_t count, const VkCommandBuffer* cmds,
                         VkFence fence) {
    Submit(count, cmds, queue_, fence);
}

void CommandPool::Submit(const std::vector<VkCommandBuffer>& cmds,
                         VkFence fence) {
    Submit(cmds.size(), cmds.data(), queue_, fence);
}

void CommandPool::SubmitAndWait(size_t count, const VkCommandBuffer* cmds,
                                VkQueue queue) {
    Submit(count, cmds, queue);
    VkResult result = vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device_, command_pool_, (uint32_t)count, cmds);
}

void CommandPool::SubmitAndWait(const std::vector<VkCommandBuffer>& cmds,
                                VkQueue queue) {
    SubmitAndWait(cmds.size(), cmds.data(), queue);
}

void CommandPool::SubmitAndWait(VkCommandBuffer cmds, VkQueue queue) {
    SubmitAndWait(1, &cmds, queue);
}

void CommandPool::SubmitAndWait(VkCommandBuffer cmds) {
    SubmitAndWait(1, &cmds, queue_);
}

void CommandPool::SubmitAndWait(const std::vector<VkCommandBuffer>& cmds) {
    SubmitAndWait(cmds.size(), cmds.data(), queue_);
}

void CommandPool::SubmitAndWait(size_t count, const VkCommandBuffer* cmds) {
    SubmitAndWait(count, cmds, queue_);
}

}  // namespace toystation
