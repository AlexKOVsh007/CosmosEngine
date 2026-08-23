#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Context;

// Блокнот, из которого выдаются command buffer'ы.
class Commands {
public:
    explicit Commands(const Context& context);
    ~Commands();

    Commands(const Commands&) = delete;
    Commands& operator=(const Commands&) = delete;
    Commands(Commands&&) = delete;
    Commands& operator=(Commands&&) = delete;

    VkCommandPool getPool() const { return pool; }

    // Освобождать не нужно: буферы умирают вместе с пулом.
    std::vector<VkCommandBuffer> allocate(uint32_t count) const;

private:
    // Доступ без владения.
    const Context* context = nullptr;

    VkCommandPool pool = VK_NULL_HANDLE;
};
