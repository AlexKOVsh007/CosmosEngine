#pragma once

#include "render/Frame.hpp"

#include <vulkan/vulkan.h>

#include <cstddef>
#include <vector>

class Commands;
class Context;
class Framebuffers;
class Pipeline;
class RenderPass;
class Swapchain;

// Рисует и показывает кадры.
class Renderer {
public:
    Renderer(const Context& context, const Swapchain& swapchain,
             const RenderPass& renderPass, const Framebuffers& framebuffers,
             const Pipeline& pipeline, const Commands& commands);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void drawFrame();

private:
    void recordCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

    // Доступ без владения.
    const Context* context = nullptr;
    const Swapchain* swapchain = nullptr;
    const RenderPass* renderPass = nullptr;
    const Framebuffers* framebuffers = nullptr;
    const Pipeline* pipeline = nullptr;

    std::vector<Frame> frames;
    size_t currentFrame = 0;
};
