#include "render/Frame.hpp"

#include "vk/Commands.hpp"
#include "vk/Context.hpp"

#include <stdexcept>
#include <utility>

Frame::Frame(const Context& ctx, const Commands& commands) : context(&ctx) {
    commandBuffer = commands.allocate(1).front();

    const VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    // Поднятым: иначе первый кадр ждал бы сигнала, которого некому подать.
    const VkFenceCreateInfo fenceInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VkDevice device = ctx.getDevice();
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailable) != VK_SUCCESS ||
        vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinished) != VK_SUCCESS ||
        vkCreateFence(device, &fenceInfo, nullptr, &inFlight) != VK_SUCCESS) {
        throw std::runtime_error("не удалось создать объекты синхронизации кадра");
    }
}

Frame::~Frame() {
    destroy();
}

void Frame::destroy() noexcept {
    VkDevice device = context->getDevice();
    vkDestroySemaphore(device, imageAvailable, nullptr);
    vkDestroySemaphore(device, renderFinished, nullptr);
    vkDestroyFence(device, inFlight, nullptr);

    imageAvailable = VK_NULL_HANDLE;
    renderFinished = VK_NULL_HANDLE;
    inFlight = VK_NULL_HANDLE;
}

Frame::Frame(Frame&& other) noexcept
    : context(other.context),
      commandBuffer(std::exchange(other.commandBuffer, VK_NULL_HANDLE)),
      imageAvailable(std::exchange(other.imageAvailable, VK_NULL_HANDLE)),
      renderFinished(std::exchange(other.renderFinished, VK_NULL_HANDLE)),
      inFlight(std::exchange(other.inFlight, VK_NULL_HANDLE)) {}

Frame& Frame::operator=(Frame&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        commandBuffer = std::exchange(other.commandBuffer, VK_NULL_HANDLE);
        imageAvailable = std::exchange(other.imageAvailable, VK_NULL_HANDLE);
        renderFinished = std::exchange(other.renderFinished, VK_NULL_HANDLE);
        inFlight = std::exchange(other.inFlight, VK_NULL_HANDLE);
    }
    return *this;
}
