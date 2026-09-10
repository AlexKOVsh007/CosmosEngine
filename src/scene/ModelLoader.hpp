#pragma once

#include "scene/ModelData.hpp"

#include <string>

// Единственная точка входа для сцены: формат выбирается по расширению файла.
[[nodiscard]] ModelData loadModel(const std::string& path);
