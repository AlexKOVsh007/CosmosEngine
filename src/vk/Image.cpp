#include "vk/Image.hpp"

#include "vk/Allocator.hpp"
#include "vk/Buffer.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"

// Реализация stb_image разворачивается ровно здесь и больше нигде.
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <algorithm>
#include <array>
#include <bit>
#include <stdexcept>
#include <utility>

Image::Image(const Context& ctx, const Allocator& allocatorRef, uint32_t width,
             uint32_t height, uint32_t levels, VkFormat imageFormat,
             VkImageUsageFlags usage, VkImageAspectFlags aspect)
    : context(&ctx), allocator(&allocatorRef), format(imageFormat), mipLevels(levels) {
    const VkImageCreateInfo imageInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = imageFormat,
        .extent{.width = width, .height = height, .depth = 1},
        .mipLevels = levels,
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
            .levelCount = levels,
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

    return Image(context, allocator, width, height, 1, format,
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

Image Image::texture(const Context& context, const Allocator& allocator,
                     const Commands& commands, const std::string& path) {
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        throw std::runtime_error("не удалось прочитать картинку: " + path);
    }

    const VkDeviceSize bytes = static_cast<VkDeviceSize>(width) * height * 4;

    // Сколько раз сторона делится пополам: bit_width считает это в целых числах.
    const uint32_t levels =
        std::bit_width(static_cast<uint32_t>(std::max(width, height)));

    Buffer staging = Buffer::staging(allocator, bytes);
    staging.write(pixels, static_cast<size_t>(bytes));
    stbi_image_free(pixels);

    Image result(context, allocator, static_cast<uint32_t>(width),
                 static_cast<uint32_t>(height), levels, VK_FORMAT_R8G8B8A8_SRGB,
                 VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT,
                 VK_IMAGE_ASPECT_COLOR_BIT);

    result.transitionLayout(commands, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    result.copyFrom(commands, staging, static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height));

    // Построение уровней само переводит image в layout для чтения шейдером.
    result.generateMipmaps(commands, static_cast<uint32_t>(width),
                           static_cast<uint32_t>(height));

    return result;
}

void Image::transitionLayout(const Commands& commands, VkImageLayout oldLayout,
                             VkImageLayout newLayout) const {
    VkCommandBuffer commandBuffer = commands.beginSingleTime();

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    } else {
        throw std::runtime_error("переход раскладки не описан");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0,
                         nullptr, 1, &barrier);

    commands.endSingleTime(commandBuffer);
}

void Image::copyFrom(const Commands& commands, const Buffer& source, uint32_t width,
                     uint32_t height) const {
    VkCommandBuffer commandBuffer = commands.beginSingleTime();

    const VkBufferImageCopy region{
        .imageSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageExtent{.width = width, .height = height, .depth = 1},
    };

    vkCmdCopyBufferToImage(commandBuffer, source.getHandle(), image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    commands.endSingleTime(commandBuffer);
}

void Image::generateMipmaps(const Commands& commands, uint32_t width,
                            uint32_t height) const {
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(context->getPhysicalDevice(), format, &properties);
    if ((properties.optimalTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == 0) {
        throw std::runtime_error("устройство не умеет фильтровать этот формат");
    }

    VkCommandBuffer commandBuffer = commands.beginSingleTime();

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    int32_t mipWidth = static_cast<int32_t>(width);
    int32_t mipHeight = static_cast<int32_t>(height);

    for (uint32_t level = 1; level < mipLevels; ++level) {
        // Предыдущий уровень дописан — переводим его в источник копирования.
        barrier.subresourceRange.baseMipLevel = level - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                             &barrier);

        // Уменьшение вдвое делает сама видеокарта, с линейной фильтрацией.
        const VkImageBlit blit{
            .srcSubresource{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = level - 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .srcOffsets{{0, 0, 0}, {mipWidth, mipHeight, 1}},
            .dstSubresource{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = level,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .dstOffsets{{0, 0, 0},
                        {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1,
                         1}},
        };

        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        // Уровень-источник больше не нужен для копирования — отдаём шейдеру.
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                             nullptr, 1, &barrier);

        if (mipWidth > 1) {
            mipWidth /= 2;
        }
        if (mipHeight > 1) {
            mipHeight /= 2;
        }
    }

    // Последний уровень никто не уменьшал, он всё ещё приёмник копирования.
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    commands.endSingleTime(commandBuffer);
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
      format(std::exchange(other.format, VK_FORMAT_UNDEFINED)),
      mipLevels(std::exchange(other.mipLevels, 1)) {}

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        allocator = other.allocator;
        image = std::exchange(other.image, VK_NULL_HANDLE);
        allocation = std::exchange(other.allocation, VK_NULL_HANDLE);
        view = std::exchange(other.view, VK_NULL_HANDLE);
        format = std::exchange(other.format, VK_FORMAT_UNDEFINED);
        mipLevels = std::exchange(other.mipLevels, 1);
    }
    return *this;
}
