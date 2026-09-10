#pragma once

#include "scene/ModelData.hpp"

#include <string>

// Чтение формата OBJ. О Vulkan здесь не знают ничего.
namespace obj {

// Материал необязателен: без .mtl рядом модель приедет без текстуры.
[[nodiscard]] ModelData load(const std::string& path);

}  // namespace obj
