#include "CosmosEngine.hpp"

#include <chrono>

namespace {

constexpr uint32_t windowWidth = 800;
constexpr uint32_t windowHeight = 600;

// По набору дескрипторов на каждый кадр в полёте.
constexpr uint32_t framesInFlight = 1;

}  // namespace

CosmosEngine::CosmosEngine()
    : window(windowWidth, windowHeight, "Cosmos Engine"),
      camera({2.0f, 2.0f, 2.0f}, {0.0f, 0.0f, 0.0f}),
      input(window),
      context(window),
      allocator(context),
      swapchain(context),
      depthImage(Image::depth(context, allocator, swapchain.getExtent().width,
                              swapchain.getExtent().height)),
      renderPass(context, swapchain.getImageFormat(), depthImage.getFormat()),
      framebuffers(context, renderPass, swapchain, depthImage),
      descriptors(context, framesInFlight),
      vertexShader(context, "shaders/vert.spv"),
      fragmentShader(context, "shaders/frag.spv"),
      pipeline(context, renderPass, vertexShader, fragmentShader, descriptors),
      commands(context),
      mesh(allocator, commands, primitives::pyramid()),
      texture(context, allocator, commands, "textures/pottery_basecolor.jpg"),
      renderer(context, swapchain, renderPass, framebuffers, pipeline, mesh, commands,
               allocator, descriptors, texture, camera) {}

void CosmosEngine::run() {
    auto previous = std::chrono::steady_clock::now();

    while (!window.shouldClose()) {
        const auto now = std::chrono::steady_clock::now();
        const float delta = std::chrono::duration<float>(now - previous).count();
        previous = now;

        window.pollEvents();
        input.apply(camera, delta);

        if (renderer.drawFrame()) {
            recreateSwapchain();
        }
    }
}

void CosmosEngine::recreateSwapchain() {
    window.waitWhileMinimized();

    // Трогать ресурсы, пока видеокарта их использует, нельзя.
    vkDeviceWaitIdle(context.getDevice());

    swapchain = Swapchain(context, swapchain.getHandle());
    depthImage = Image::depth(context, allocator, swapchain.getExtent().width,
                              swapchain.getExtent().height);
    framebuffers = Framebuffers(context, renderPass, swapchain, depthImage);
}
