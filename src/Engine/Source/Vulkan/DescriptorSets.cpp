#include "DescriptorSets.h"

void toystation::DescriptorSetBindings::AddBinding(
    uint32_t binding, VkDescriptorType type, uint32_t count,
    VkShaderStageFlags stage_flags, const VkSampler* sampler) {
    VkDescriptorSetLayoutBinding layout_binding{binding, type, count,
                                                stage_flags, sampler};
    bindings_.push_back(layout_binding);
}

void toystation::DescriptorSetBindings::SetBindingFlags(
    uint32_t binding, VkDescriptorBindingFlags binding_flags) {
    for (size_t i = 0; i < bindings_.size(); i++) {
        if (bindings_[i].binding == binding) {
            if (binding_flags_.size() <= i) {
                binding_flags_.resize(i + 1, 0);
            }
            binding_flags_[i] = binding_flags;
            return;
        }
    }
}

void toystation::DescriptorSetBindings::Clear() {
    binding_flags_.clear();
    bindings_.clear();
}

VkDescriptorSetLayout toystation::DescriptorSetBindings::CreateLayout(
    VkContext* ctx, VkDescriptorSetLayoutCreateFlags flags) const {
    VkDescriptorSetLayoutBindingFlagsCreateInfo bindings_create_info{};
    bindings_create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindings_create_info.bindingCount = uint32_t(binding_flags_.size());
    bindings_create_info.pBindingFlags = binding_flags_.data();

    VkDescriptorSetLayoutCreateInfo layout_create_info{};
    layout_create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.bindingCount = bindings_.size();
    layout_create_info.pBindings = bindings_.data();
    layout_create_info.flags = flags;
    layout_create_info.pNext =
        binding_flags_.empty() ? nullptr : &bindings_create_info;

    VkDescriptorSetLayout layout = nullptr;
    ctx->CreateDescriptorSetLayout(layout_create_info, layout);
    return layout;
}

void toystation::DescriptorSetBindings::AddRequiredPoolSizes(
    std::vector<VkDescriptorPoolSize>& pool_size, uint32_t num_sets) {
    for (auto iter = bindings_.cbegin(); iter != bindings_.cend(); ++iter) {
        bool found = false;
        for (auto itpool = pool_size.begin(); itpool != pool_size.end();
             ++itpool) {
            if (itpool->type == iter->descriptorType) {
                itpool->descriptorCount += iter->descriptorCount * num_sets;
                found = true;
                break;
            }
        }
        if (!found) {
            VkDescriptorPoolSize pool;
            pool.type = iter->descriptorType;
            pool.descriptorCount = iter->descriptorCount * num_sets;
            pool_size.push_back(pool);
        }
    }
}

VkDescriptorPool toystation::DescriptorSetBindings::CreatePool(
    VkContext* ctx, uint32_t max_sets, VkDescriptorPoolCreateFlags flags) {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    AddRequiredPoolSizes(pool_sizes, max_sets);

    VkDescriptorPool descr_pool = nullptr;
    VkDescriptorPoolCreateInfo descr_pool_info = {};
    descr_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descr_pool_info.pNext = nullptr;
    descr_pool_info.maxSets = max_sets;
    descr_pool_info.poolSizeCount = uint32_t(pool_sizes.size());
    descr_pool_info.pPoolSizes = pool_sizes.data();
    descr_pool_info.flags = flags;

    // scene pool
    vkCreateDescriptorPool(ctx->GetDevice(), &descr_pool_info, nullptr,
                           &descr_pool);
    return descr_pool;
}

VkWriteDescriptorSet toystation::DescriptorSetBindings::MakeWrite(
    VkDescriptorSet dst_set, uint32_t binding, uint32_t array_element) const {
    VkWriteDescriptorSet write_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    for (size_t i = 0; i < bindings_.size(); i++) {
        if (bindings_[i].binding == binding) {
            write_set.descriptorCount = 1;
            write_set.descriptorType = bindings_[i].descriptorType;
            write_set.dstBinding = binding;
            write_set.dstSet = dst_set;
            write_set.dstArrayElement = array_element;
            return write_set;
        }
    }
    assert(0 && "binding not found");
    return write_set;
}

VkWriteDescriptorSet toystation::DescriptorSetBindings::MakeWrite(
    VkDescriptorSet dst_set, uint32_t binding,
    const VkDescriptorImageInfo* image_info, uint32_t array_element) const {
    VkWriteDescriptorSet write_set = MakeWrite(dst_set, binding, array_element);
    assert(write_set.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER ||
           write_set.descriptorType ==
               VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
           write_set.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
           write_set.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
           write_set.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
    write_set.pImageInfo = image_info;
    return write_set;
}

VkWriteDescriptorSet toystation::DescriptorSetBindings::MakeWrite(
    VkDescriptorSet dst_set, uint32_t binding,
    const VkDescriptorBufferInfo* buffer_info, uint32_t array_element) const {
    VkWriteDescriptorSet write_set = MakeWrite(dst_set, binding, array_element);
    assert(
        write_set.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
        write_set.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
        write_set.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
        write_set.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
    write_set.pBufferInfo = buffer_info;
    return write_set;
}

VkWriteDescriptorSet toystation::DescriptorSetBindings::MakeWrite(
    VkDescriptorSet dst_set, uint32_t binding, const VkBufferView* buffer_view,
    uint32_t array_element) const {
    VkWriteDescriptorSet write_set = MakeWrite(dst_set, binding, array_element);
    assert(write_set.descriptorType ==
               VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
           write_set.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    write_set.pTexelBufferView = buffer_view;
    return write_set;
}

toystation::DescriptorSetContainer::DescriptorSetContainer() : ctx_(nullptr) {}

void toystation::DescriptorSetContainer::Init(VkContext* ctx) { ctx_ = ctx; }

void toystation::DescriptorSetContainer::DeInit() {
    if (pipeline_layout_) {
        vkDestroyPipelineLayout(ctx_->GetDevice(), pipeline_layout_, nullptr);
    }
    if (layout_) {
        vkDestroyDescriptorSetLayout(ctx_->GetDevice(), layout_, nullptr);
    }
    descriptor_sets_.clear();
    if (pool_) {
        vkDestroyDescriptorPool(ctx_->GetDevice(), pool_, nullptr);
    }
}

void toystation::DescriptorSetContainer::AddBinding(
    uint32_t binding, VkDescriptorType type, uint32_t count,
    VkShaderStageFlags stage_flags, const VkSampler* sampler) {
    bindings_.AddBinding(binding, type, count, stage_flags, sampler);
}

void toystation::DescriptorSetContainer::AddBinding(
    VkDescriptorSetLayoutBinding layout_binding) {
    bindings_.AddBinding(layout_binding.binding, layout_binding.descriptorType,
                         layout_binding.descriptorCount,
                         layout_binding.stageFlags,
                         layout_binding.pImmutableSamplers);
}

VkDescriptorSetLayout toystation::DescriptorSetContainer::InitLayout(
    VkDescriptorSetLayoutCreateFlags flags) {
    assert(ctx_ && "ctx is nullptr,call init first");
    layout_ = bindings_.CreateLayout(ctx_, flags);
    return layout_;
}

VkDescriptorPool toystation::DescriptorSetContainer::InitPool(
    uint32_t max_sets) {
    pool_ = bindings_.CreatePool(ctx_, max_sets);
    AllocDescriptorSets(max_sets, descriptor_sets_);
    return pool_;
}

VkPipelineLayout toystation::DescriptorSetContainer::InitPipelineLayout(
    uint32_t num_ranges, const VkPushConstantRange* ranges,
    VkPipelineLayoutCreateFlags flags) {
    VkPipelineLayoutCreateInfo layout_create_info;
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    layout_create_info.setLayoutCount = 1;
    layout_create_info.pSetLayouts = &layout_;
    layout_create_info.pushConstantRangeCount = num_ranges;
    layout_create_info.pPushConstantRanges = ranges;
    layout_create_info.flags = flags;

    vkCreatePipelineLayout(ctx_->GetDevice(), &layout_create_info, nullptr,
                           &pipeline_layout_);
    return pipeline_layout_;
}

VkDescriptorSet& toystation::DescriptorSetContainer::GetSet(uint32_t set_idx) {
    return descriptor_sets_[set_idx];
}

VkWriteDescriptorSet toystation::DescriptorSetContainer::MakeWrite(
    uint32_t set_idx, uint32_t binding, const VkDescriptorImageInfo* image_info,
    uint32_t array_element) {
    return bindings_.MakeWrite(GetSet(set_idx), binding, image_info,
                               array_element);
}

VkWriteDescriptorSet toystation::DescriptorSetContainer::MakeWrite(
    uint32_t set_idx, uint32_t binding,
    const VkDescriptorBufferInfo* buffer_info, uint32_t array_element) {
    return bindings_.MakeWrite(GetSet(set_idx), binding, buffer_info,
                               array_element);
}

VkWriteDescriptorSet toystation::DescriptorSetContainer::MakeWrite(
    uint32_t set_idx, uint32_t binding, const VkBufferView* buffer_view,
    uint32_t array_element) {
    return bindings_.MakeWrite(GetSet(set_idx), binding, buffer_view,
                               array_element);
}

void toystation::DescriptorSetContainer::UpdateSets(
    std::vector<VkWriteDescriptorSet>& sets) {
        
    vkUpdateDescriptorSets(ctx_->GetDevice(), sets.size(), sets.data(), 0,
                           nullptr);
}

void toystation::DescriptorSetContainer::AllocDescriptorSets(
    uint32_t num_allocated, std::vector<VkDescriptorSet>& sets) {
    sets.resize(num_allocated);

    std::vector<VkDescriptorSetLayout> layouts(num_allocated, layout_);
    VkDescriptorSetAllocateInfo alloc_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    alloc_info.descriptorPool = pool_;
    alloc_info.descriptorSetCount = num_allocated;
    alloc_info.pSetLayouts = layouts.data();

    vkAllocateDescriptorSets(ctx_->GetDevice(), &alloc_info, sets.data());
}
