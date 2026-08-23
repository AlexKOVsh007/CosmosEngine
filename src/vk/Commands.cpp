#include "vk/Commands.hpp"

#include "vk/Context.hpp"

#include <stdexcept>

Commands::Commands(const Context& ctx) : context(&ctx) {
    const VkCommandPoolCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        // Разрешаем стирать и записывать буфер заново каждый кадр.
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = ctx.getQueueFamily(),
    };

    if (vkCreateCommandPool(ctx.getDevice(), &info, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateCommandPool failed");
    }
}

Commands::~Commands() {
    vkDestroyCommandPool(context->getDevice(), pool, nullptr);
}

std::vector<VkCommandBuffer> Commands::allocate(uint32_t count) const {
    const VkCommandBufferAllocateInfo info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = pool,
        // PRIMARY отправляется в очередь сам; SECONDARY только вызывается из него.
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count,
    };

    std::vector<VkCommandBuffer> buffers(count);
    if (vkAllocateCommandBuffers(context->getDevice(), &info, buffers.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers failed");
    }
    return buffers;
}
