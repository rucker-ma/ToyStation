#pragma once
#include "VkContext.h"

namespace toystation {
class DescriptorSetBindings {
public:
    DescriptorSetBindings() = default;
    void AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count,
                    VkShaderStageFlags stage_flags,
                    const VkSampler* sampler = nullptr);
    void SetBindingFlags(uint32_t binding,
                         VkDescriptorBindingFlags binding_flags);
    void Clear();
    VkDescriptorSetLayout CreateLayout(
        VkContext* ctx, VkDescriptorSetLayoutCreateFlags flags = 0) const;
    void AddRequiredPoolSizes(std::vector<VkDescriptorPoolSize>& pool_size,
                              uint32_t num_sets);
    VkDescriptorPool CreatePool(VkContext* ctx, uint32_t max_sets = 1,
                                VkDescriptorPoolCreateFlags flags = 0);

    VkWriteDescriptorSet MakeWrite(VkDescriptorSet dst_set, uint32_t binding,
                                   uint32_t array_element = 0) const;
    VkWriteDescriptorSet MakeWrite(VkDescriptorSet dst_set, uint32_t binding,
                                   const VkDescriptorImageInfo* image_info,
                                   uint32_t array_element = 0) const;

    VkWriteDescriptorSet MakeWrite(VkDescriptorSet dst_set, uint32_t binding,
                                   const VkDescriptorBufferInfo* buffer_info,
                                   uint32_t array_element = 0) const;
    VkWriteDescriptorSet MakeWrite(VkDescriptorSet dst_set, uint32_t binding,
                                   const VkBufferView* buffer_view,
                                   uint32_t array_element = 0) const;

private:
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
    std::vector<VkDescriptorBindingFlags> binding_flags_;
};

class DescriptorSetContainer {
public:
    DescriptorSetContainer();
    void Init(VkContext* ctx);
    void DeInit();
    void AddBinding(uint32_t binding, VkDescriptorType type, uint32_t count,
                    VkShaderStageFlags stage_flags,
                    const VkSampler* sampler = nullptr);
    void AddBinding(VkDescriptorSetLayoutBinding layout_binding);
    VkDescriptorSetLayout InitLayout(
        VkDescriptorSetLayoutCreateFlags flags = 0);
    VkDescriptorPool InitPool(uint32_t max_sets);
    VkPipelineLayout InitPipelineLayout(
        uint32_t num_ranges, const VkPushConstantRange* ranges = nullptr,
        VkPipelineLayoutCreateFlags flags = 0);

    VkDescriptorSet& GetSet(uint32_t set_idx);
    VkWriteDescriptorSet MakeWrite(uint32_t set_idx, uint32_t binding,
                                   const VkDescriptorImageInfo* image_info,
                                   uint32_t array_element = 0);

    VkWriteDescriptorSet MakeWrite(uint32_t set_idx, uint32_t binding,
                                   const VkDescriptorBufferInfo* buffer_info,
                                   uint32_t array_element = 0);
    VkWriteDescriptorSet MakeWrite(uint32_t set_idx, uint32_t binding,
                                   const VkBufferView* buffer_view,
                                   uint32_t array_element = 0);
    void UpdateSets(std::vector<VkWriteDescriptorSet>& sets);

private:
    void AllocDescriptorSets(uint32_t num_allocated,
                             std::vector<VkDescriptorSet>& sets);

private:
    VkContext* ctx_;
    DescriptorSetBindings bindings_;
    VkDescriptorSetLayout layout_;
    VkDescriptorPool pool_;
    VkPipelineLayout pipeline_layout_;
    std::vector<VkDescriptorSet> descriptor_sets_;
};
}  // namespace toystation