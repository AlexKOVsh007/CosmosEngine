#include "CosmosEngine.hpp"

#include <array>
#include <chrono>

namespace {

constexpr uint32_t windowWidth = 800;
constexpr uint32_t windowHeight = 600;

// По набору дескрипторов на каждый кадр в полёте.
constexpr uint32_t framesInFlight = 1;

// Пирамида: четыре угла основания и вершина.
constexpr std::array<Vertex, 5> pyramidVertices{
    Vertex{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.15f}, {0.0f, 0.0f}},
    Vertex{{0.5f, -0.5f, 0.0f}, {1.0f, 0.25f, 0.0f}, {1.0f, 0.0f}},
    Vertex{{0.5f, 0.5f, 0.0f}, {0.35f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    Vertex{{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.55f}, {0.0f, 1.0f}},
    Vertex{{0.0f, 0.0f, 0.8f}, {0.0f, 1.0f, 0.65f}, {0.5f, 0.5f}},
};

// Обход против часовой стрелки при взгляде снаружи — иначе грань отсекут.
constexpr std::array<uint32_t, 18> pyramidIndices{
    0, 2, 1, 0, 3, 2,  // основание
    0, 1, 4,           // боковые грани
    1, 2, 4,           //
    2, 3, 4,           //
    3, 0, 4,           //
};

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
      mesh(allocator, commands, pyramidVertices, pyramidIndices),
      texture(context, allocator, commands, "textures/pottery.jpg"),
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
