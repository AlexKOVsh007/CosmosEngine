#include "render/Splash.hpp"

#include "vk/Allocator.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"
#include "vk/Image.hpp"
#include "vk/Swapchain.hpp"

#include <limits>
#include <stdexcept>

namespace splash {
namespace {

// Заставка рисуется один раз и синхронно, поэтому стадии не уточняются.
void transition(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout from,
                VkImageLayout to) {
    const VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT,
        .oldLayout = from,
        .newLayout = to,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };

    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
}

}  // namespace

void show(const Context& context, const Allocator& allocator, const Commands& commands,
          const Swapchain& swapchain, const std::string& path) {
    // Не всякий surface разрешает копировать в свои картинки — тогда обходимся без заставки.
    if ((swapchain.getImageUsage() & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0) {
        return;
    }

    const Image logo = Image::texture(context, allocator, commands, path);

    // Fence вместо семафора: следующий кадр начнётся только после нашего ожидания.
    const VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    VkFence acquired = VK_NULL_HANDLE;
    if (vkCreateFence(context.getDevice(), &fenceInfo, nullptr, &acquired) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateFence failed");
    }

    uint32_t index = 0;
    const VkResult result =
        vkAcquireNextImageKHR(context.getDevice(), swapchain.getHandle(),
                              std::numeric_limits<uint64_t>::max(), VK_NULL_HANDLE,
                              acquired, &index);

    if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
        vkWaitForFences(context.getDevice(), 1, &acquired, VK_TRUE,
                        std::numeric_limits<uint64_t>::max());

        const VkExtent2D source = logo.getExtent();
        const VkExtent2D target = swapchain.getExtent();

        // Blit, а не copy: он умеет растягивать логотип под размер окна.
        const VkImageBlit region{
            .srcSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1},
            .srcOffsets{{0, 0, 0},
                        {static_cast<int32_t>(source.width),
                         static_cast<int32_t>(source.height), 1}},
            .dstSubresource{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .mipLevel = 0,
                            .baseArrayLayer = 0,
                            .layerCount = 1},
            .dstOffsets{{0, 0, 0},
                        {static_cast<int32_t>(target.width),
                         static_cast<int32_t>(target.height), 1}},
        };

        VkCommandBuffer commandBuffer = commands.beginSingleTime();

        transition(commandBuffer, logo.getHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        transition(commandBuffer, swapchain.getImages()[index], VK_IMAGE_LAYOUT_UNDEFINED,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vkCmdBlitImage(commandBuffer, logo.getHandle(),
                       VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchain.getImages()[index],
                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);

        transition(commandBuffer, swapchain.getImages()[index],
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        commands.endSingleTime(commandBuffer);

        const VkSwapchainKHR handles[]{swapchain.getHandle()};
        const VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = handles,
            .pImageIndices = &index,
        };
        vkQueuePresentKHR(context.getGraphicsQueue(), &presentInfo);
    }

    vkDestroyFence(context.getDevice(), acquired, nullptr);
}

}  // namespace splash
