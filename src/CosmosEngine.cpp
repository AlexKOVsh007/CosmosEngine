#include "CosmosEngine.hpp"

namespace {

constexpr uint32_t windowWidth = 800;
constexpr uint32_t windowHeight = 600;

}  // namespace

CosmosEngine::CosmosEngine()
    : window(windowWidth, windowHeight, "Cosmos Engine"),
      context(window),
      swapchain(context),
      renderPass(context, swapchain.getImageFormat()),
      framebuffers(context, renderPass, swapchain),
      vertexShader(context, "shaders/vert.spv"),
      fragmentShader(context, "shaders/frag.spv"),
      pipeline(context, renderPass, vertexShader, fragmentShader),
      commands(context),
      renderer(context, swapchain, renderPass, framebuffers, pipeline, commands) {}

void CosmosEngine::run() {
    while (!window.shouldClose()) {
        window.pollEvents();
        renderer.drawFrame();
    }
}
