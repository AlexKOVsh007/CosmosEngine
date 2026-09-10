#include "CosmosEngine.hpp"

#include "render/Splash.hpp"
#include "scene/ModelLoader.hpp"

#include <chrono>
#include <thread>

namespace {

constexpr uint32_t windowWidth = 800;
constexpr uint32_t windowHeight = 600;

// По набору дескрипторов на каждый кадр в полёте.
constexpr uint32_t framesInFlight = 1;

constexpr const char* modelPath = "models/scan/model.obj";

// Формат может не нести материала вовсе — тогда красим вот этим.
constexpr const char* fallbackTexture = "textures/gold/basecolor.jpg";

constexpr const char* splashLogo = "assets/logo.png";

// Нижняя граница показа: у лёгкой модели заставка иначе просто мелькнула бы.
constexpr auto splashMinimum = std::chrono::milliseconds(500);

// Заставка показывается до разбора модели: он занимает заметное время.
ModelData loadBehindSplash(const Window& window, const Context& context,
                           const Allocator& allocator, const Commands& commands,
                           const Swapchain& swapchain, const std::string& path) {
    window.show();
    splash::show(context, allocator, commands, swapchain, splashLogo);

    const auto shown = std::chrono::steady_clock::now();
    ModelData model = loadModel(path);

    // Точка в прошлом — возврат мгновенный, так что долгая загрузка не удлиняется.
    std::this_thread::sleep_until(shown + splashMinimum);
    return model;
}

const std::string& textureOf(const ModelData& model) {
    static const std::string fallback = fallbackTexture;
    return model.baseColorTexture.empty() ? fallback : model.baseColorTexture;
}

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
      model(loadBehindSplash(window, context, allocator, commands, swapchain, modelPath)),
      mesh(allocator, commands, model.mesh),
      texture(context, allocator, commands, textureOf(model)),
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
