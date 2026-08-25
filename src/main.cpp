#include "CosmosEngine.hpp"

#include <cstdlib>
#include <iostream>

int main() {
    try {
        CosmosEngine engine;
        engine.run();
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
