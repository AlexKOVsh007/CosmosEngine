#pragma once

#include "scene/MeshData.hpp"
#include "vk/Buffer.hpp"

#include <cstdint>
#include <span>

class Allocator;
class Commands;

// Геометрия в памяти видеокарты: вершины и порядок их обхода.
class Mesh {
public:
    Mesh(const Allocator& allocator, const Commands& commands, const MeshData& data);

    Mesh(const Allocator& allocator, const Commands& commands,
         std::span<const Vertex> vertices, std::span<const uint32_t> indices);

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = default;
    Mesh& operator=(Mesh&&) noexcept = default;

    VkBuffer getVertexBuffer() const { return vertexBuffer.getHandle(); }
    VkBuffer getIndexBuffer() const { return indexBuffer.getHandle(); }
    uint32_t getIndexCount() const { return indexCount; }

    glm::vec3 getCenter() const { return center; }
    glm::vec3 getExtent() const { return extent; }

private:
    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t indexCount = 0;

    // Габаритная коробка: по ней сцена приводит модель к своим единицам.
    glm::vec3 center{0.0f};
    glm::vec3 extent{1.0f};
};
