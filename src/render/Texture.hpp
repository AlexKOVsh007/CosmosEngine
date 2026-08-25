#pragma once

#include "vk/Image.hpp"

#include <string>

class Allocator;
class Commands;
class Context;

// Картинка вместе с правилами её чтения: фильтрация, края, анизотропия.
class Texture {
public:
    Texture(const Context& context, const Allocator& allocator, const Commands& commands,
            const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    VkImageView getView() const { return image.getView(); }
    VkSampler getSampler() const { return sampler; }

private:
    void destroy() noexcept;

    // Доступ без владения.
    const Context* context = nullptr;

    Image image;
    VkSampler sampler = VK_NULL_HANDLE;
};
