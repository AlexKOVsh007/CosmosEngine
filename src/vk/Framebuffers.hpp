#pragma once

#include <vulkan/vulkan.h>

#include <cstddef>
#include <vector>

class Context;
class RenderPass;
class Swapchain;

// Связывает image views swapchain с render pass: по framebuffer на каждый кадр.
class Framebuffers {
public:
    Framebuffers(const Context& context, const RenderPass& renderPass,
                 const Swapchain& swapchain);
    ~Framebuffers();

    Framebuffers(const Framebuffers&) = delete;
    Framebuffers& operator=(const Framebuffers&) = delete;

    // Пересоздаются вместе со swapchain — присваиванием нового объекта.
    Framebuffers(Framebuffers&& other) noexcept;
    Framebuffers& operator=(Framebuffers&& other) noexcept;

    size_t size() const { return framebuffers.size(); }
    VkFramebuffer operator[](size_t index) const { return framebuffers[index]; }

private:
    void destroy() noexcept;

    // Доступ без владения.
    const Context* context = nullptr;

    std::vector<VkFramebuffer> framebuffers;
};
