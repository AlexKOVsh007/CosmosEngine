#include "scene/GltfLoader.hpp"

// Картинки грузит движок сам, поэтому реализацию stb_image tinygltf не разворачивает.
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace gltf {
namespace {

// Имена атрибутов заданы спецификацией glTF; хвост _0 — первый набор UV из нескольких.
constexpr const char* positionAttribute = "POSITION";
constexpr const char* normalAttribute = "NORMAL";
constexpr const char* texCoordAttribute = "TEXCOORD_0";

// Данные accessor'а: bufferView задаёт кусок файла, accessor — место внутри куска.
const unsigned char* accessorBytes(const tinygltf::Model& model,
                                   const tinygltf::Accessor& accessor) {
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    return model.buffers[view.buffer].data.data() + view.byteOffset + accessor.byteOffset;
}

// glm::vec3 — те же три float подряд, что и в файле, поэтому копируется целиком.
std::vector<glm::vec3> readVec3(const tinygltf::Model& model,
                                const tinygltf::Accessor& accessor) {
    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
        accessor.type != TINYGLTF_TYPE_VEC3) {
        throw std::runtime_error("ожидался VEC3 из float");
    }

    const unsigned char* bytes = accessorBytes(model, accessor);
    const int stride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

    std::vector<glm::vec3> values(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i) {
        std::memcpy(&values[i], bytes + i * static_cast<size_t>(stride), sizeof(glm::vec3));
    }
    return values;
}

std::vector<glm::vec2> readVec2(const tinygltf::Model& model,
                                const tinygltf::Accessor& accessor) {
    if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
        accessor.type != TINYGLTF_TYPE_VEC2) {
        throw std::runtime_error("ожидался VEC2 из float");
    }

    const unsigned char* bytes = accessorBytes(model, accessor);
    const int stride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

    std::vector<glm::vec2> values(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i) {
        std::memcpy(&values[i], bytes + i * static_cast<size_t>(stride), sizeof(glm::vec2));
    }
    return values;
}

// Экспортёр выбирает минимальную разрядность, а VkBuffer у нас всегда 32-битный.
std::vector<uint32_t> readIndices(const tinygltf::Model& model,
                                  const tinygltf::Primitive& primitive) {
    if (primitive.indices < 0) {
        throw std::runtime_error("примитив без индексов пока не поддерживается");
    }

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    const unsigned char* bytes = accessorBytes(model, accessor);
    const int stride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);

    std::vector<uint32_t> indices(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i) {
        // memcpy, а не разыменование: данные в файле не обязаны быть выровнены.
        const unsigned char* element = bytes + i * static_cast<size_t>(stride);
        switch (accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                uint32_t value = 0;
                std::memcpy(&value, element, sizeof(value));
                indices[i] = value;
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                uint16_t value = 0;
                std::memcpy(&value, element, sizeof(value));
                indices[i] = value;
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                indices[i] = *element;
                break;
            default:
                throw std::runtime_error("неизвестная разрядность индексов в модели");
        }
    }
    return indices;
}

// .glb — тот же формат, упакованный в один двоичный файл вместо пары.
bool isBinary(const std::string& path) {
    return path.ends_with(".glb");
}

tinygltf::Model openFile(const std::string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string error;
    std::string warning;

    const bool loaded =
        isBinary(path) ? loader.LoadBinaryFromFile(&model, &error, &warning, path)
                       : loader.LoadASCIIFromFile(&model, &error, &warning, path);

    if (!loaded) {
        throw std::runtime_error("не удалось загрузить модель " + path + ": " +
                                 (error.empty() ? "файл не найден" : error));
    }
    if (model.meshes.empty()) {
        throw std::runtime_error("в модели нет ни одного меша: " + path);
    }
    return model;
}

// Атрибуты адресуются строками, а набор их у каждого экспортёра свой.
const tinygltf::Accessor& findAttribute(const tinygltf::Model& model,
                                        const tinygltf::Primitive& primitive,
                                        const std::string& name) {
    const auto found = primitive.attributes.find(name);
    if (found == primitive.attributes.end()) {
        throw std::runtime_error("в модели нет атрибута " + name);
    }
    return model.accessors[found->second];
}

// Карта цвета — единственная, которую сейчас читает фрагментный шейдер.
std::string findBaseColorTexture(const tinygltf::Model& model,
                                 const tinygltf::Primitive& primitive,
                                 const std::string& modelPath) {
    if (primitive.material < 0) {
        throw std::runtime_error("у примитива нет материала: " + modelPath);
    }

    const tinygltf::Material& material = model.materials[primitive.material];
    const int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
    if (textureIndex < 0) {
        throw std::runtime_error("в материале нет карты цвета: " + modelPath);
    }

    const std::string& uri = model.images[model.textures[textureIndex].source].uri;
    if (uri.empty()) {
        throw std::runtime_error("карта цвета встроена в файл, а не лежит рядом: " + modelPath);
    }

    // Путь в файле указан относительно самого файла модели.
    return (std::filesystem::path(modelPath).parent_path() / uri).string();
}

}  // namespace

ModelData load(const std::string& path) {
    const tinygltf::Model model = openFile(path);
    const tinygltf::Primitive& primitive = model.meshes.front().primitives.front();

    const std::vector<glm::vec3> positions =
        readVec3(model, findAttribute(model, primitive, positionAttribute));
    const std::vector<glm::vec3> normals =
        readVec3(model, findAttribute(model, primitive, normalAttribute));
    const std::vector<glm::vec2> texCoords =
        readVec2(model, findAttribute(model, primitive, texCoordAttribute));

    if (normals.size() != positions.size() || texCoords.size() != positions.size()) {
        throw std::runtime_error("атрибуты вершин расходятся по количеству: " + path);
    }

    ModelData result;
    result.mesh.vertices.resize(positions.size());
    for (size_t i = 0; i < positions.size(); ++i) {
        // Три ленты из файла сходятся здесь в одну вершину: SoA превращается в AoS.
        result.mesh.vertices[i].position = positions[i];
        result.mesh.vertices[i].normal = normals[i];
        result.mesh.vertices[i].texCoord = texCoords[i];
    }

    result.mesh.indices = readIndices(model, primitive);
    result.baseColorTexture = findBaseColorTexture(model, primitive, path);
    return result;
}

}  // namespace gltf
