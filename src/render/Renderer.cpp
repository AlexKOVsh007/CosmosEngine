#include "render/Renderer.hpp"

#include "render/Mesh.hpp"
#include "render/Uniforms.hpp"
#include "vk/Allocator.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"
#include "vk/Descriptors.hpp"
#include "vk/Framebuffers.hpp"
#include "vk/Pipeline.hpp"
#include "vk/RenderPass.hpp"
#include "vk/Swapchain.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <limits>
#include <stdexcept>

namespace {

constexpr VkClearValue clearColor{{{0.05f, 0.07f, 0.16f, 1.0f}}};
constexpr uint32_t framesInFlight = 1;

}  // namespace

Renderer::Renderer(const Context& ctx, const Swapchain& swapchainRef,
                   const RenderPass& renderPassRef, const Framebuffers& framebuffersRef,
                   const Pipeline& pipelineRef, const Mesh& meshRef,
                   const Commands& commands, const Allocator& allocator,
                   const Descriptors& descriptors)
    : context(&ctx),
      swapchain(&swapchainRef),
      renderPass(&renderPassRef),
      framebuffers(&framebuffersRef),
      pipeline(&pipelineRef),
      mesh(&meshRef),
      startTime(std::chrono::steady_clock::now()) {
    frames.reserve(framesInFlight);
    for (uint32_t i = 0; i < framesInFlight; ++i) {
        frames.emplace_back(ctx, commands, allocator, descriptors);
    }
}

Renderer::~Renderer() {
    // Уничтожать ресурсы, пока видеокарта их использует, нельзя.
    vkDeviceWaitIdle(context->getDevice());
}

void Renderer::drawFrame() {
    constexpr uint64_t noTimeout = std::numeric_limits<uint64_t>::max();

    const Frame& frame = frames[currentFrame];
    VkDevice device = context->getDevice();
    VkFence inFlight = frame.getInFlight();

    vkWaitForFences(device, 1, &inFlight, VK_TRUE, noTimeout);

    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(device, swapchain->getHandle(), noTimeout,
                          frame.getImageAvailable(), VK_NULL_HANDLE, &imageIndex);

    vkResetFences(device, 1, &inFlight);

    updateUniforms(frame);

    VkCommandBuffer commandBuffer = frame.getCommandBuffer();
    vkResetCommandBuffer(commandBuffer, 0);
    recordCommands(commandBuffer, imageIndex);

    const VkSemaphore waitSemaphores[]{frame.getImageAvailable()};
    const VkSemaphore signalSemaphores[]{frame.getRenderFinished()};
    // Ждём только перед записью цвета: предыдущие стадии могут идти раньше.
    const VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    const VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores,
    };

    if (vkQueueSubmit(context->getGraphicsQueue(), 1, &submitInfo, inFlight) !=
        VK_SUCCESS) {
        throw std::runtime_error("vkQueueSubmit failed");
    }

    const VkSwapchainKHR swapchains[]{swapchain->getHandle()};
    const VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex,
    };

    vkQueuePresentKHR(context->getGraphicsQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % frames.size();
}

void Renderer::recordCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex) const {
    const VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("vkBeginCommandBuffer failed");
    }

    const VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass->getHandle(),
        .framebuffer = (*framebuffers)[imageIndex],
        .renderArea{
            .offset{0, 0},
            .extent = swapchain->getExtent(),
        },
        .clearValueCount = 1,
        .pClearValues = &clearColor,
    };

    // Заливка происходит здесь: она описана в render pass как loadOp.
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->getHandle());

    const VkExtent2D extent = swapchain->getExtent();
    const VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    const VkRect2D scissor{.offset{0, 0}, .extent = extent};

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    const VkDescriptorSet set = frames[currentFrame].getDescriptorSet();
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->getLayout(), 0, 1, &set, 0, nullptr);

    const VkBuffer vertexBuffers[]{mesh->getVertexBuffer()};
    const VkDeviceSize offsets[]{0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, mesh->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, mesh->getIndexCount(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer failed");
    }
}

void Renderer::updateUniforms(const Frame& frame) const {
    const float seconds =
        std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

    const VkExtent2D extent = swapchain->getExtent();
    const float aspect = static_cast<float>(extent.width) / static_cast<float>(extent.height);

    UniformBufferObject uniforms{
        .model = glm::rotate(glm::mat4(1.0f), seconds * glm::radians(45.0f),
                             glm::vec3(0.0f, 0.0f, 1.0f)),
        .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)),
        .projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 10.0f),
    };

    // GLM считает по правилам OpenGL, где ось Y экрана направлена вверх.
    uniforms.projection[1][1] *= -1.0f;

    frame.getUniformBuffer().write(&uniforms, sizeof(uniforms));
}
