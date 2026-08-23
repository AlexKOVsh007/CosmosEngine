#pragma once

#include <vulkan/vulkan.h>

class Commands;
class Context;

// Ресурсы одного кадра: команды и синхронизация.
class Frame {
public:
    Frame(const Context& context, const Commands& commands);
    ~Frame();

    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

    VkCommandBuffer getCommandBuffer() const { return commandBuffer; }
    VkSemaphore getImageAvailable() const { return imageAvailable; }
    VkSemaphore getRenderFinished() const { return renderFinished; }
    VkFence getInFlight() const { return inFlight; }

private:
    void destroy() noexcept;

    // Доступ без владения.
    const Context* context = nullptr;

    // Освобождается вместе с пулом.
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    // Ждут внутри видеокарты, процессору не видны.
    VkSemaphore imageAvailable = VK_NULL_HANDLE;
    VkSemaphore renderFinished = VK_NULL_HANDLE;

    // Через него процессор узнаёт, что видеокарта закончила.
    VkFence inFlight = VK_NULL_HANDLE;
};
