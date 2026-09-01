#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <array>
#include <cstddef>

// Одна вершина модели. Раскладка полей должна совпадать с описанием ниже.
struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
    // Направление наружу от поверхности: по нему считается освещённость.
    glm::vec3 normal;

    // Как читать ленту вершин: с каким шагом и по вершине или по экземпляру.
    static VkVertexInputBindingDescription bindingDescription() {
        return VkVertexInputBindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    // Где внутри вершины лежит каждый вход шейдера; связь идёт по номеру location.
    static std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions() {
        return std::array<VkVertexInputAttributeDescription, 4>{
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position),
            },
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color),
            },
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord),
            },
            VkVertexInputAttributeDescription{
                .location = 3,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, normal),
            },
        };
    }
};
