#pragma once

#include <vulkan/vulkan.h>

class Buffer;
class Context;
class Texture;

// Описывает, какие ресурсы видит шейдер, и выдаёт наборы с конкретными ресурсами.
class Descriptors {
public:
    Descriptors(const Context& context, uint32_t maxSets);
    ~Descriptors();

    Descriptors(const Descriptors&) = delete;
    Descriptors& operator=(const Descriptors&) = delete;
    Descriptors(Descriptors&&) = delete;
    Descriptors& operator=(Descriptors&&) = delete;

    VkDescriptorSetLayout getLayout() const { return layout; }

    // Освобождать не нужно: наборы умирают вместе с пулом.
    VkDescriptorSet allocate() const;

    // Связывает набор с конкретным буфером по номеру, который знает шейдер.
    void bindUniform(VkDescriptorSet set, uint32_t binding, const Buffer& buffer) const;

    // Картинка и правила чтения приходят в шейдер одной записью.
    void bindTexture(VkDescriptorSet set, uint32_t binding,
                     const Texture& texture) const;

private:
    // Доступ без владения.
    const Context* context = nullptr;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool pool = VK_NULL_HANDLE;
};
