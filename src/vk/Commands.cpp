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

VkCommandBuffer Commands::beginSingleTime() const {
    VkCommandBuffer commandBuffer = allocate(1).front();

    const VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        // Драйвер знает, что буфер одноразовый, и не тратится на переиспользование.
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Commands::endSingleTime(VkCommandBuffer commandBuffer) const {
    vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };

    vkQueueSubmit(context->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    // Разовая операция: проще дождаться очереди, чем заводить fence.
    vkQueueWaitIdle(context->getGraphicsQueue());

    vkFreeCommandBuffers(context->getDevice(), pool, 1, &commandBuffer);
}
