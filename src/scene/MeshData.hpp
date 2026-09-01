#pragma once

#include "scene/Vertex.hpp"

#include <cstdint>
#include <vector>

// Геометрия в оперативной памяти: то, что читают из файла или строят сами.
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
