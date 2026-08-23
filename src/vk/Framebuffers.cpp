#include "vk/Framebuffers.hpp"

#include "vk/Context.hpp"
#include "vk/RenderPass.hpp"
#include "vk/Swapchain.hpp"

#include <stdexcept>
#include <utility>

Framebuffers::Framebuffers(const Context& ctx, const RenderPass& renderPass,
                           const Swapchain& swapchain)
    : context(&ctx) {
    const std::vector<VkImageView>& views = swapchain.getImageViews();
    const VkExtent2D extent = swapchain.getExtent();

    framebuffers.resize(views.size());

    for (size_t i = 0; i < views.size(); ++i) {
        // Порядок attachment'ов обязан совпадать с порядком в render pass.
        const VkFramebufferCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass.getHandle(),
            .attachmentCount = 1,
            .pAttachments = &views[i],
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(ctx.getDevice(), &info, nullptr, &framebuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("vkCreateFramebuffer failed");
        }
    }
}

Framebuffers::~Framebuffers() {
    destroy();
}

void Framebuffers::destroy() noexcept {
    for (VkFramebuffer framebuffer : framebuffers) {
        vkDestroyFramebuffer(context->getDevice(), framebuffer, nullptr);
    }
    framebuffers.clear();
}

Framebuffers::Framebuffers(Framebuffers&& other) noexcept
    : context(other.context), framebuffers(std::move(other.framebuffers)) {}

Framebuffers& Framebuffers::operator=(Framebuffers&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        framebuffers = std::move(other.framebuffers);
    }
    return *this;
}
