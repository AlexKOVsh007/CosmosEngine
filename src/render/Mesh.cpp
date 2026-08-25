#include "render/Mesh.hpp"

Mesh::Mesh(const Allocator& allocator, const Commands& commands,
           std::span<const Vertex> vertices, std::span<const uint32_t> indices)
    : vertexBuffer(Buffer::vertexFrom(allocator, commands, vertices.data(),
                                      vertices.size_bytes())),
      indexBuffer(
          Buffer::indexFrom(allocator, commands, indices.data(), indices.size_bytes())),
      indexCount(static_cast<uint32_t>(indices.size())) {}
