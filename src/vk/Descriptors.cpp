#include "vk/Descriptors.hpp"

#include "vk/Buffer.hpp"
#include "render/Texture.hpp"
#include "vk/Context.hpp"

#include <array>
#include <stdexcept>

Descriptors::Descriptors(const Context& ctx, uint32_t maxSets) : context(&ctx) {
    const VkDescriptorSetLayoutBinding uniformBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        // Матрицы нужны вершинному, позиции света и камеры — фрагментному.
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    const VkDescriptorSetLayoutBinding samplerBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    const std::array bindings{uniformBinding, samplerBinding};

    const VkDescriptorSetLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data(),
    };

    if (vkCreateDescriptorSetLayout(ctx.getDevice(), &layoutInfo, nullptr, &layout) !=
        VK_SUCCESS) {
        throw std::runtime_error("vkCreateDescriptorSetLayout failed");
    }

    // Пул резервирует места заранее: сколько наборов и сколько ресурсов в них.
    const std::array poolSizes{
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSets},
        VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSets},
    };

    const VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = maxSets,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
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

void Descriptors::bindTexture(VkDescriptorSet set, uint32_t binding,
                              const Texture& texture) const {
    const VkDescriptorImageInfo imageInfo{
        .sampler = texture.getSampler(),
        .imageView = texture.getView(),
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    const VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = set,
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &imageInfo,
    };

    vkUpdateDescriptorSets(context->getDevice(), 1, &write, 0, nullptr);
}
