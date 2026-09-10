#include "vk/Swapchain.hpp"

#include "vk/Context.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

// SRGB — цвета, как их видит глаз; иначе первый доступный.
VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR>& available) {
    for (const VkSurfaceFormatKHR& format : available) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return available.front();
}

}  // namespace

Swapchain::Swapchain(const Context& ctx, VkSwapchainKHR oldSwapchain) : context(&ctx) {
    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx.getPhysicalDevice(),
                                              ctx.getSurface(), &caps);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.getPhysicalDevice(), ctx.getSurface(),
                                         &formatCount, nullptr);
    if (formatCount == 0) {
        throw std::runtime_error("surface не предлагает ни одного формата");
    }
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx.getPhysicalDevice(), ctx.getSurface(),
                                         &formatCount, formats.data());

    const VkSurfaceFormatKHR chosen = chooseFormat(formats);
    imageFormat = chosen.format;
    extent = caps.currentExtent;

    // TRANSFER_DST нужен заставке; surface не обязан его разрешать, поэтому спрашиваем.
    imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if ((caps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) != 0) {
        imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    // Минимум + 1: с ровным минимумом программа простаивала бы в ожидании кадра.
    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        imageCount = caps.maxImageCount;
    }

    const VkSwapchainCreateInfoKHR createInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = ctx.getSurface(),
        .minImageCount = imageCount,
        .imageFormat = chosen.format,
        .imageColorSpace = chosen.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = imageUsage,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        // FIFO поддерживается всегда — вертикальная синхронизация.
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchain,
    };

    if (vkCreateSwapchainKHR(ctx.getDevice(), &createInfo, nullptr, &swapchain) !=
        VK_SUCCESS) {
        throw std::runtime_error("vkCreateSwapchainKHR failed");
    }

    // Просили «не меньше», поэтому реальное число спрашиваем, а не считаем.
    uint32_t actualCount = 0;
    vkGetSwapchainImagesKHR(ctx.getDevice(), swapchain, &actualCount, nullptr);
    images.resize(actualCount);
    vkGetSwapchainImagesKHR(ctx.getDevice(), swapchain, &actualCount, images.data());

    createImageViews();

    std::cout << "Swapchain: " << extent.width << "x" << extent.height << ", кадров "
              << actualCount << " (просили " << imageCount << "), формат "
              << imageFormat << std::endl;
}

void Swapchain::createImageViews() {
    imageViews.resize(images.size());

    for (size_t i = 0; i < images.size(); ++i) {
        const VkImageViewCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageFormat,
            // Смотрим на цвет, единственный mip-уровень, единственный слой.
            .subresourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        if (vkCreateImageView(context->getDevice(), &info, nullptr, &imageViews[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("vkCreateImageView failed");
        }
    }
}

Swapchain::~Swapchain() {
    destroy();
}

void Swapchain::destroy() noexcept {
    if (swapchain == VK_NULL_HANDLE) {
        return;
    }

    for (VkImageView view : imageViews) {
        vkDestroyImageView(context->getDevice(), view, nullptr);
    }
    imageViews.clear();

    vkDestroySwapchainKHR(context->getDevice(), swapchain, nullptr);
    swapchain = VK_NULL_HANDLE;
    images.clear();
}

Swapchain::Swapchain(Swapchain&& other) noexcept
    : context(other.context),
      swapchain(std::exchange(other.swapchain, VK_NULL_HANDLE)),
      imageFormat(other.imageFormat),
      extent(other.extent),
      images(std::move(other.images)),
      imageViews(std::move(other.imageViews)) {}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        swapchain = std::exchange(other.swapchain, VK_NULL_HANDLE);
        imageFormat = other.imageFormat;
        extent = other.extent;
        images = std::move(other.images);
        imageViews = std::move(other.imageViews);
    }
    return *this;
}
