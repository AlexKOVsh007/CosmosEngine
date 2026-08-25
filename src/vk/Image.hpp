#pragma once

#include <vk_mem_alloc.h>

#include <cstdint>

class Allocator;
class Context;

// Картинка в памяти видеокарты вместе со своим view.
class Image {
public:
    // Буфер глубины: формат подбирается из поддерживаемых устройством.
    [[nodiscard]] static Image depth(const Context& context, const Allocator& allocator,
                                     uint32_t width, uint32_t height);

    Image() = default;
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    VkImage getHandle() const { return image; }
    VkImageView getView() const { return view; }
    VkFormat getFormat() const { return format; }

    // Первый из списка, который устройство поддерживает для нужного применения.
    static VkFormat findSupportedFormat(const Context& context,
                                        const VkFormat* candidates, uint32_t count,
                                        VkFormatFeatureFlags features);

private:
    Image(const Context& context, const Allocator& allocator, uint32_t width,
          uint32_t height, VkFormat format, VkImageUsageFlags usage,
          VkImageAspectFlags aspect);

    void destroy() noexcept;

    // Доступ без владения.
    const Context* context = nullptr;
    const Allocator* allocator = nullptr;

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
};
