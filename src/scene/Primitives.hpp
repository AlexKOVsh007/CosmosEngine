#pragma once

#include "scene/MeshData.hpp"

// Простые фигуры, построенные кодом. О Vulkan здесь не знают ничего.
namespace primitives {

// Квадратное основание и вершина; грани плоские, рёбра чёткие.
MeshData pyramid();

}  // namespace primitives
