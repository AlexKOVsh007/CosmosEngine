#include "CosmosEngine.hpp"

#include <array>

namespace {

constexpr uint32_t windowWidth = 800;
constexpr uint32_t windowHeight = 600;

// По набору дескрипторов на каждый кадр в полёте.
constexpr uint32_t framesInFlight = 1;

// Порядок обхода — по часовой стрелке: ось Y в Vulkan направлена вниз.
constexpr std::array<Vertex, 3> triangleVertices{
    Vertex{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.5f, 0.0f}},
    Vertex{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
    Vertex{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
};

constexpr std::array<uint32_t, 3> triangleIndices{0, 1, 2};

}  // namespace

CosmosEngine::CosmosEngine()
    : window(windowWidth, windowHeight, "Cosmos Engine"),
      context(window),
      allocator(context),
      swapchain(context),
      renderPass(context, swapchain.getImageFormat()),
      framebuffers(context, renderPass, swapchain),
      descriptors(context, framesInFlight),
      vertexShader(context, "shaders/vert.spv"),
      fragmentShader(context, "shaders/frag.spv"),
      pipeline(context, renderPass, vertexShader, fragmentShader, descriptors),
      commands(context),
      mesh(allocator, commands, triangleVertices, triangleIndices),
      renderer(context, swapchain, renderPass, framebuffers, pipeline, mesh, commands,
               allocator, descriptors) {}

void CosmosEngine::run() {
    while (!window.shouldClose()) {
        window.pollEvents();
        renderer.drawFrame();
    }
}
