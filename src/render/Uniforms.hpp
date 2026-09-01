#pragma once

// Vulkan держит глубину в диапазоне 0..1, в отличие от OpenGL с -1..1.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// Порядок полей обязан совпадать с GLSL: матрицы выровнены по 16 байт сами.
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    // vec4, а не vec3: по std140 трёхкомпонентный всё равно занял бы 16 байт.
    glm::vec4 lightPosition;
    glm::vec4 cameraPosition;
};
