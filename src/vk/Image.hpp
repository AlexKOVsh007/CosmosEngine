#pragma once

#include <vk_mem_alloc.h>

#include <cstdint>
#include <string>

class Allocator;
class Buffer;
class Commands;
class Context;

// Картинка в памяти видеокарты вместе со своим view.
class Image {
public:
    // Буфер глубины: формат подбирается из поддерживаемых устройством.
    [[nodiscard]] static Image depth(const Context& context, const Allocator& allocator,
                                     uint32_t width, uint32_t height);

    // Текстура из файла: распаковка, загрузка и построение mip-уровней.
    [[nodiscard]] static Image texture(const Context& context, const Allocator& allocator,
                                       const Commands& commands, const std::string& path);

    Image() = default;
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    VkImage getHandle() const { return image; }
    VkImageView getView() const { return view; }
    VkFormat getFormat() const { return format; }
    uint32_t getMipLevels() const { return mipLevels; }
    VkExtent2D getExtent() const { return extent; }

    // Первый из списка, который устройство поддерживает для нужного применения.
    static VkFormat findSupportedFormat(const Context& context,
                                        const VkFormat* candidates, uint32_t count,
                                        VkFormatFeatureFlags features);

private:
    Image(const Context& context, const Allocator& allocator, uint32_t width,
          uint32_t height, uint32_t mipLevels, VkFormat format, VkImageUsageFlags usage,
          VkImageAspectFlags aspect);

    void transitionLayout(const Commands& commands, VkImageLayout oldLayout,
                          VkImageLayout newLayout) const;
    void copyFrom(const Commands& commands, const Buffer& source, uint32_t width,
                  uint32_t height) const;
    void generateMipmaps(const Commands& commands, uint32_t width, uint32_t height) const;

    void destroy() noexcept;

    // Доступ без владения.
    const Context* context = nullptr;
    const Allocator* allocator = nullptr;

    VkImage image = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t mipLevels = 1;
    VkExtent2D extent{};
};
