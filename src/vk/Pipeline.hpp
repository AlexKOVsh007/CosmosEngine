#pragma once

#include <vulkan/vulkan.h>

class Context;
class Descriptors;
class RenderPass;
class Shader;

// Зафиксированная конфигурация рисования: шейдеры, растеризация, смешивание.
class Pipeline {
public:
    Pipeline(const Context& context, const RenderPass& renderPass, const Shader& vertex,
             const Shader& fragment, const Descriptors& descriptors);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;

    VkPipeline getHandle() const { return pipeline; }
    VkPipelineLayout getLayout() const { return layout; }

private:
    // Доступ без владения.
    const Context* context = nullptr;

    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
};
