#pragma once

#include "scene/ModelData.hpp"

#include <string>

// Чтение формата glTF. О Vulkan здесь не знают ничего.
namespace gltf {

// Первый примитив первого меша; иерархия узлов пока не читается.
[[nodiscard]] ModelData load(const std::string& path);

}  // namespace gltf
