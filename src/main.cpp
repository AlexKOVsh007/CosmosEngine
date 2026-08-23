#include "core/Window.hpp"
#include "render/Renderer.hpp"
#include "vk/Commands.hpp"
#include "vk/Context.hpp"
#include "vk/Framebuffers.hpp"
#include "vk/RenderPass.hpp"
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
        Commands commands(context);
        Renderer renderer(context, swapchain, renderPass, framebuffers, commands);

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
