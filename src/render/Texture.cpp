#include "render/Texture.hpp"

#include "vk/Context.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

Texture::Texture(const Context& ctx, const Allocator& allocator, const Commands& commands,
                 const std::string& path)
    : context(&ctx), image(Image::texture(ctx, allocator, commands, path)) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(ctx.getPhysicalDevice(), &properties);

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(ctx.getPhysicalDevice(), &features);

    const VkSamplerCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        // LINEAR: смешивать соседние тексели, иначе картинка будет ступенчатой.
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        // Анизотропия спасает грани, видимые под острым углом.
        .anisotropyEnable = features.samplerAnisotropy ? VK_TRUE : VK_FALSE,
        .maxAnisotropy = features.samplerAnisotropy ? properties.limits.maxSamplerAnisotropy
                                                    : 1.0f,
        .compareEnable = VK_FALSE,
        .minLod = 0.0f,
        .maxLod = static_cast<float>(image.getMipLevels()),
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
    };

    std::cout << "Текстура: " << path << ", mip-уровней " << image.getMipLevels()
              << ", анизотропия " << info.maxAnisotropy << std::endl;

    if (vkCreateSampler(ctx.getDevice(), &info, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateSampler failed");
    }
}

Texture::~Texture() {
    destroy();
}

void Texture::destroy() noexcept {
    if (sampler != VK_NULL_HANDLE) {
        vkDestroySampler(context->getDevice(), sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }
}

Texture::Texture(Texture&& other) noexcept
    : context(other.context),
      image(std::move(other.image)),
      sampler(std::exchange(other.sampler, VK_NULL_HANDLE)) {}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        image = std::move(other.image);
        sampler = std::exchange(other.sampler, VK_NULL_HANDLE);
    }
    return *this;
}
