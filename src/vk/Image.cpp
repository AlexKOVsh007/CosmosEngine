#include "vk/Image.hpp"

#include "vk/Allocator.hpp"
#include "vk/Context.hpp"

#include <array>
#include <stdexcept>
#include <utility>

Image::Image(const Context& ctx, const Allocator& allocatorRef, uint32_t width,
             uint32_t height, VkFormat imageFormat, VkImageUsageFlags usage,
             VkImageAspectFlags aspect)
    : context(&ctx), allocator(&allocatorRef), format(imageFormat) {
    const VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = imageFormat,
        .extent{.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        // OPTIMAL: раскладку выбирает драйвер, читать её напрямую нельзя.
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    const VmaAllocationCreateInfo allocInfo{.usage = VMA_MEMORY_USAGE_AUTO};

    if (vmaCreateImage(allocator->getHandle(), &imageInfo, &allocInfo, &image, &allocation,
                       nullptr) != VK_SUCCESS) {
        throw std::runtime_error("vmaCreateImage failed");
    }

    const VkImageViewCreateInfo viewInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageFormat,
        .subresourceRange{
            .aspectMask = aspect,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    if (vkCreateImageView(ctx.getDevice(), &viewInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateImageView failed");
    }
}

VkFormat Image::findSupportedFormat(const Context& context, const VkFormat* candidates,
                                    uint32_t count, VkFormatFeatureFlags features) {
    for (uint32_t i = 0; i < count; ++i) {
        VkFormatProperties properties{};
        vkGetPhysicalDeviceFormatProperties(context.getPhysicalDevice(), candidates[i],
                                            &properties);
        if ((properties.optimalTilingFeatures & features) == features) {
            return candidates[i];
        }
    }
    throw std::runtime_error("устройство не поддерживает ни один из форматов");
}

Image Image::depth(const Context& context, const Allocator& allocator, uint32_t width,
                   uint32_t height) {
    constexpr std::array candidates{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                    VK_FORMAT_D24_UNORM_S8_UINT};

    const VkFormat format =
        findSupportedFormat(context, candidates.data(),
                            static_cast<uint32_t>(candidates.size()),
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    return Image(context, allocator, width, height, format,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

Image::~Image() {
    destroy();
}

void Image::destroy() noexcept {
    if (view != VK_NULL_HANDLE) {
        vkDestroyImageView(context->getDevice(), view, nullptr);
        view = VK_NULL_HANDLE;
    }
    if (image != VK_NULL_HANDLE) {
        vmaDestroyImage(allocator->getHandle(), image, allocation);
        image = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }
}

Image::Image(Image&& other) noexcept
    : context(other.context),
      allocator(other.allocator),
      image(std::exchange(other.image, VK_NULL_HANDLE)),
      allocation(std::exchange(other.allocation, VK_NULL_HANDLE)),
      view(std::exchange(other.view, VK_NULL_HANDLE)),
      format(std::exchange(other.format, VK_FORMAT_UNDEFINED)) {}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        allocator = other.allocator;
        image = std::exchange(other.image, VK_NULL_HANDLE);
        allocation = std::exchange(other.allocation, VK_NULL_HANDLE);
        view = std::exchange(other.view, VK_NULL_HANDLE);
        format = std::exchange(other.format, VK_FORMAT_UNDEFINED);
    }
    return *this;
}
