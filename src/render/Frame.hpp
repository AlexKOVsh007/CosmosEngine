#pragma once

#include "vk/Buffer.hpp"

#include <vulkan/vulkan.h>

class Allocator;
class Commands;
class Context;
class Descriptors;

// Ресурсы одного кадра: команды и синхронизация.
class Frame {
public:
    Frame(const Context& context, const Commands& commands,
          const Allocator& allocator, const Descriptors& descriptors);
    ~Frame();

    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    Frame(Frame&& other) noexcept;
    Frame& operator=(Frame&& other) noexcept;

    VkCommandBuffer getCommandBuffer() const { return commandBuffer; }
    VkSemaphore getImageAvailable() const { return imageAvailable; }
    VkSemaphore getRenderFinished() const { return renderFinished; }
    VkFence getInFlight() const { return inFlight; }
    const Buffer& getUniformBuffer() const { return uniformBuffer; }
    VkDescriptorSet getDescriptorSet() const { return descriptorSet; }

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

    // Матрицы обновляются каждый кадр, поэтому у кадра свои.
    Buffer uniformBuffer;
    // Освобождается вместе с пулом дескрипторов.
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};
