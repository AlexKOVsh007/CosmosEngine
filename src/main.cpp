#include "core/Window.hpp"
#include "vk/Context.hpp"
#include "vk/Swapchain.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    try {
        Window window(800, 600, "Cosmos Engine");
        Context context(window);
        Swapchain swapchain(context);

        while (!window.shouldClose()) {
            window.pollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
