#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Context;

// Очередь images, в которые рисуем по очереди и показываем на экране.
class Swapchain {
public:
    // На один surface нельзя два swapchain — новый объявляется преемником старого.
    explicit Swapchain(const Context& context,
                       VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    // Пересоздание при изменении размера окна — присваивание нового объекта.
    Swapchain(Swapchain&& other) noexcept;
    Swapchain& operator=(Swapchain&& other) noexcept;

    VkSwapchainKHR getHandle() const { return swapchain; }
    VkFormat getImageFormat() const { return imageFormat; }
    VkExtent2D getExtent() const { return extent; }
    const std::vector<VkImage>& getImages() const { return images; }
    const std::vector<VkImageView>& getImageViews() const { return imageViews; }

private:
    void createImageViews();
    void destroy() noexcept;

    // Доступ без владения; указатель, а не ссылка, — иначе нет move-присваивания.
    const Context* context = nullptr;

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    VkFormat imageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D extent{};

    // Принадлежат swapchain: создаются и уничтожаются вместе с ним.
    std::vector<VkImage> images;
    // А эти создали мы — значит и уничтожать нам.
    std::vector<VkImageView> imageViews;
};
