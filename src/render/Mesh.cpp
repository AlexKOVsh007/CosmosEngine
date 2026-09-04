#include "render/Mesh.hpp"

Mesh::Mesh(const Allocator& allocator, const Commands& commands, const MeshData& data)
    : Mesh(allocator, commands, data.vertices, data.indices) {}

Mesh::Mesh(const Allocator& allocator, const Commands& commands,
           std::span<const Vertex> vertices, std::span<const uint32_t> indices)
    : vertexBuffer(Buffer::vertexFrom(allocator, commands, vertices.data(),
                                      vertices.size_bytes())),
      indexBuffer(
          Buffer::indexFrom(allocator, commands, indices.data(), indices.size_bytes())),
      indexCount(static_cast<uint32_t>(indices.size())) {
    if (vertices.empty()) {
        return;
    }

    glm::vec3 low = vertices.front().position;
    glm::vec3 high = low;
    for (const Vertex& vertex : vertices) {
        low = glm::min(low, vertex.position);
        high = glm::max(high, vertex.position);
    }

    center = (low + high) * 0.5f;
    extent = high - low;
}
