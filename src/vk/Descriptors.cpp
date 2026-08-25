#include "vk/Descriptors.hpp"

#include "vk/Buffer.hpp"
#include "vk/Context.hpp"

#include <stdexcept>

Descriptors::Descriptors(const Context& ctx, uint32_t maxSets) : context(&ctx) {
    const VkDescriptorSetLayoutBinding uniformBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uniformBinding,
    };

    if (vkCreateDescriptorSetLayout(ctx.getDevice(), &layoutInfo, nullptr, &layout) !=
        VK_SUCCESS) {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed");
    }

    // Пул резервирует места заранее: сколько наборов и сколько ресурсов в них.
    const VkDescriptorPoolSize poolSize{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = maxSets,
    };

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = maxSets,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
    };

    if (vkCreateDescriptorPool(ctx.getDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateDescriptorPool failed");
    }
}

Descriptors::~Descriptors() {
    vkDestroyDescriptorPool(context->getDevice(), pool, nullptr);
    vkDestroyDescriptorSetLayout(context->getDevice(), layout, nullptr);
}

VkDescriptorSet Descriptors::allocate() const {
    const VkDescriptorSetAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };

    VkDescriptorSet set = VK_NULL_HANDLE;
    if (vkAllocateDescriptorSets(context->getDevice(), &info, &set) != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateDescriptorSets failed");
    }
    return set;
}

void Descriptors::bindUniform(VkDescriptorSet set, uint32_t binding,
                              const Buffer& buffer) const {
    const VkDescriptorBufferInfo bufferInfo{
        .buffer = buffer.getHandle(),
        .offset = 0,
        .range = buffer.getSize(),
    };

    const VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &bufferInfo,
    };

    vkUpdateDescriptorSets(context->getDevice(), 1, &write, 0, nullptr);
}
