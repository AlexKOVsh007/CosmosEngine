#pragma once

#include <vulkan/vulkan.h>

class Context;

// Какие attachment'ы участвуют в проходе и что с ними делать до и после.
class RenderPass {
public:
    RenderPass(const Context& context, VkFormat colorFormat);
    ~RenderPass();

    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass(RenderPass&&) = delete;
    RenderPass& operator=(RenderPass&&) = delete;

    VkRenderPass getHandle() const { return renderPass; }

private:
    // Доступ без владения.
    const Context* context = nullptr;

    VkRenderPass renderPass = VK_NULL_HANDLE;
};
