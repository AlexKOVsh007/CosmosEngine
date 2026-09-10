#include "scene/ModelLoader.hpp"

#include "scene/GltfLoader.hpp"
#include "scene/ObjLoader.hpp"

#include <cctype>
#include <filesystem>
#include <stdexcept>

namespace {

std::string lowerExtension(const std::string& path) {
    std::string extension = std::filesystem::path(path).extension().string();
    for (char& symbol : extension) {
        symbol = static_cast<char>(std::tolower(static_cast<unsigned char>(symbol)));
    }
    return extension;
}

}  // namespace

ModelData loadModel(const std::string& path) {
    const std::string extension = lowerExtension(path);

    // .glb — тот же glTF, упакованный в один двоичный файл.
    if (extension == ".gltf" || extension == ".glb") {
        return gltf::load(path);
    }
    if (extension == ".obj") {
        return obj::load(path);
    }
    throw std::runtime_error("неизвестный формат модели: " + path);
}
