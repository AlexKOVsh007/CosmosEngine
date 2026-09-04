#include "CosmosEngine.hpp"

#include "scene/GltfLoader.hpp"

#include <chrono>

namespace {

constexpr uint32_t windowWidth = 800;
constexpr uint32_t windowHeight = 600;

// По набору дескрипторов на каждый кадр в полёте.
constexpr uint32_t framesInFlight = 1;

constexpr const char* modelPath = "models/sylvaxe/scene.gltf";

}  // namespace

CosmosEngine::CosmosEngine()
    : window(windowWidth, windowHeight, "Cosmos Engine"),
      camera({2.5f, 2.5f, 1.0f}, {0.0f, 0.0f, 0.0f}),
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
      model(gltf::load(modelPath)),
      mesh(allocator, commands, model.mesh),
      texture(context, allocator, commands, model.baseColorTexture),
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
