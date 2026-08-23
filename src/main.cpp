#include "core/Window.hpp"
#include "render/Renderer.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"
#include "vk/Framebuffers.hpp"
#include "vk/Pipeline.hpp"
#include "vk/RenderPass.hpp"
#include "vk/Shader.hpp"
#include "vk/Swapchain.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    try {
        Window window(800, 600, "Cosmos Engine");
        Context context(window);
        Swapchain swapchain(context);
        RenderPass renderPass(context, swapchain.getImageFormat());
        Framebuffers framebuffers(context, renderPass, swapchain);

        Shader vertexShader(context, "shaders/vert.spv");
        Shader fragmentShader(context, "shaders/frag.spv");
        Pipeline pipeline(context, renderPass, vertexShader, fragmentShader);

        Commands commands(context);
        Renderer renderer(context, swapchain, renderPass, framebuffers, pipeline,
                          commands);

        while (!window.shouldClose()) {
            window.pollEvents();
            renderer.drawFrame();
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
