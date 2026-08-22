#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

class Window;   // нужна только ссылка — полное описание тут ни к чему

// Фундамент Vulkan: создаётся один раз на старте, живёт до выхода.
class Context {
public:
    explicit Context(const Window& window);
    ~Context();

    // Экземпляр единственный и никуда не переезжает — запрещено всё.
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(Context&&) = delete;

    VkInstance getInstance() const { return instance; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    uint32_t getQueueFamily() const { return queueFamily; }

private:
    void createInstance();
    void createDebugMessenger();
    void createSurface(const Window& window);
    void pickPhysicalDevice();
    void createLogicalDevice();

    // Объявлены в порядке создания; уничтожает деструктор, в обратном порядке.
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    // Не создаётся, а выбирается из имеющихся — уничтожать нечего.
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    uint32_t queueFamily = 0;

    VkDevice device = VK_NULL_HANDLE;
    // Очередь принадлежит device и умирает вместе с ним.
    VkQueue graphicsQueue = VK_NULL_HANDLE;
};
