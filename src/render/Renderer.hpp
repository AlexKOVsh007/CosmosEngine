#pragma once

#include "render/Frame.hpp"

#include <vulkan/vulkan.h>

#include <chrono>
#include <cstddef>
#include <vector>

class Allocator;
class Camera;
class Commands;
class Context;
class Descriptors;
class Framebuffers;
class Mesh;
class Pipeline;
class RenderPass;
class Swapchain;
class Texture;

// Рисует и показывает кадры.
class Renderer {
public:
    Renderer(const Context& context, const Swapchain& swapchain,
             const RenderPass& renderPass, const Framebuffers& framebuffers,
             const Pipeline& pipeline, const Mesh& mesh,
             const Commands& commands, const Allocator& allocator,
             const Descriptors& descriptors, const Texture& texture,
             const Camera& camera);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    // Возвращает true, если swapchain устарел и его пора пересоздать.
    bool drawFrame();

private:
    void recordCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;
    void updateUniforms(const Frame& frame) const;

    // Доступ без владения.
    const Context* context = nullptr;
    const Swapchain* swapchain = nullptr;
    const RenderPass* renderPass = nullptr;
    const Framebuffers* framebuffers = nullptr;
    const Pipeline* pipeline = nullptr;
    const Mesh* mesh = nullptr;
    const Camera* camera = nullptr;

    std::vector<Frame> frames;
    size_t currentFrame = 0;

    // Отсчёт для вращения модели.
    std::chrono::steady_clock::time_point startTime;
};
