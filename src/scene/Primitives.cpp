#include "scene/Primitives.hpp"

namespace primitives {
namespace {

// Вершины не переиспользуются: общая нормаль сгладила бы рёбра в градиент.
void addTriangle(MeshData& mesh, glm::vec3 first, glm::vec3 second, glm::vec3 third,
                 glm::vec2 firstUV, glm::vec2 secondUV, glm::vec2 thirdUV) {
    // Векторное произведение сторон перпендикулярно грани; порядок задаёт сторону.
    const glm::vec3 normal = glm::normalize(glm::cross(second - first, third - first));

    const auto base = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.push_back({first, firstUV, normal});
    mesh.vertices.push_back({second, secondUV, normal});
    mesh.vertices.push_back({third, thirdUV, normal});

    mesh.indices.insert(mesh.indices.end(), {base, base + 1, base + 2});
}

}  // namespace

MeshData pyramid() {
    constexpr glm::vec3 baseA{-0.5f, -0.5f, 0.0f};
    constexpr glm::vec3 baseB{0.5f, -0.5f, 0.0f};
    constexpr glm::vec3 baseC{0.5f, 0.5f, 0.0f};
    constexpr glm::vec3 baseD{-0.5f, 0.5f, 0.0f};
    constexpr glm::vec3 apex{0.0f, 0.0f, 0.8f};

    MeshData mesh;
    addTriangle(mesh, baseA, baseD, baseC, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f});
    addTriangle(mesh, baseA, baseC, baseB, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f});

    addTriangle(mesh, baseA, baseB, apex, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f});
    addTriangle(mesh, baseB, baseC, apex, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f});
    addTriangle(mesh, baseC, baseD, apex, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f});
    addTriangle(mesh, baseD, baseA, apex, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f});
    return mesh;
}

}  // namespace primitives
