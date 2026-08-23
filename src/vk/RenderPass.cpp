#include "vk/RenderPass.hpp"

#include "vk/Context.hpp"

#include <stdexcept>

RenderPass::RenderPass(const Context& ctx, VkFormat colorFormat) : context(&ctx) {
    const VkAttachmentDescription colorAttachment{
        .format = colorFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        // Заливка при загрузке плитки дешевле, чем рисование прямоугольника.
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        // Старое содержимое не нужно: всё равно зальём.
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    // Номер — индекс в pAttachments; layout — раскладка на время subpass.
    const VkAttachmentReference colorRef{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorRef,
    };

    const VkRenderPassCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    if (vkCreateRenderPass(ctx.getDevice(), &info, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateRenderPass failed");
    }
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(context->getDevice(), renderPass, nullptr);
}
