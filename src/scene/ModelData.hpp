#pragma once

#include "scene/MeshData.hpp"

#include <string>

// Модель в оперативной памяти, одинаковая для любого формата файла.
struct ModelData {
    MeshData mesh;
    // Путь к карте цвета, уже склеенный с папкой самой модели.
    std::string baseColorTexture;
};
