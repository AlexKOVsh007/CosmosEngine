#include "scene/ObjLoader.hpp"

#include <charconv>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>

namespace obj {
namespace {

// Угол треугольника: OBJ адресует позицию, UV и нормаль тремя отдельными номерами.
struct Corner {
    int position = 0;
    int texCoord = 0;
    int normal = 0;

    bool operator==(const Corner&) const = default;
};

struct CornerHash {
    size_t operator()(const Corner& corner) const noexcept {
        size_t seed = 0;
        for (const int field : {corner.position, corner.texCoord, corner.normal}) {
            // Сдвиги и константа золотого сечения: без них порядок полей терялся бы.
            seed ^= static_cast<size_t>(field) + 0x9e3779b97f4a7c15ULL + (seed << 6) +
                    (seed >> 2);
        }
        return seed;
    }
};

// Следующее слово строки; сама строка укорачивается на разобранное.
std::string_view nextWord(std::string_view& text) {
    const size_t begin = text.find_first_not_of(" \t\r");
    if (begin == std::string_view::npos) {
        text = {};
        return {};
    }

    const size_t end = text.find_first_of(" \t\r", begin);
    if (end == std::string_view::npos) {
        const std::string_view word = text.substr(begin);
        text = {};
        return word;
    }

    const std::string_view word = text.substr(begin, end - begin);
    text = text.substr(end);
    return word;
}

// from_chars вместо потоков: без локали, без выделения памяти, без объекта-потока.
float nextFloat(std::string_view& text) {
    const std::string_view word = nextWord(text);
    float value = 0.0f;
    std::from_chars(word.data(), word.data() + word.size(), value);
    return value;
}

// Формы записи: "1", "1/2", "1//3", "1/2/3" — пропущенное поле остаётся нулём.
Corner parseCorner(std::string_view text) {
    Corner corner;
    int* const fields[]{&corner.position, &corner.texCoord, &corner.normal};

    for (int field = 0; field < 3; ++field) {
        const size_t slash = text.find('/');
        const std::string_view part = text.substr(0, slash);
        if (!part.empty()) {
            std::from_chars(part.data(), part.data() + part.size(), *fields[field]);
        }
        if (slash == std::string_view::npos) {
            break;
        }
        text = text.substr(slash + 1);
    }
    return corner;
}

// Нумерация с единицы; отрицательный номер отсчитывается от конца списка.
size_t resolve(int index, size_t count) {
    const size_t resolved =
        index > 0 ? static_cast<size_t>(index - 1) : count + static_cast<size_t>(index);
    if (resolved >= count) {
        throw std::runtime_error("номер вершины за пределами списка в OBJ");
    }
    return resolved;
}

// Материал OBJ лежит в отдельном файле; нас интересует только карта цвета map_Kd.
std::string readBaseColorTexture(const std::string& modelPath,
                                 const std::string& libraryName) {
    const std::filesystem::path folder = std::filesystem::path(modelPath).parent_path();
    std::ifstream library(folder / libraryName);
    if (!library.is_open()) {
        return {};
    }

    std::string line;
    while (std::getline(library, line)) {
        std::istringstream stream(line);
        std::string tag;
        std::string value;
        if (stream >> tag >> value && tag == "map_Kd") {
            return (folder / value).string();
        }
    }
    return {};
}

Vertex makeVertex(const Corner& corner, const std::vector<glm::vec3>& positions,
                  const std::vector<glm::vec2>& texCoords,
                  const std::vector<glm::vec3>& normals) {
    Vertex vertex{};
    if (corner.position != 0) {
        vertex.position = positions[resolve(corner.position, positions.size())];
    }
    if (corner.texCoord != 0) {
        vertex.texCoord = texCoords[resolve(corner.texCoord, texCoords.size())];
    }
    if (corner.normal != 0) {
        vertex.normal = normals[resolve(corner.normal, normals.size())];
    }
    return vertex;
}

}  // namespace

ModelData load(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("не удалось открыть модель " + path);
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    ModelData result;
    // Одинаковая тройка номеров — та же вершина: файл уже сказал это за нас.
    std::unordered_map<Corner, uint32_t, CornerHash> welded;

    const auto emit = [&](const Corner& corner) {
        const auto [entry, added] =
            welded.try_emplace(corner, static_cast<uint32_t>(result.mesh.vertices.size()));
        if (added) {
            result.mesh.vertices.push_back(
                makeVertex(corner, positions, texCoords, normals));
        }
        result.mesh.indices.push_back(entry->second);
    };

    std::vector<Corner> corners;
    std::string line;
    while (std::getline(file, line)) {
        std::string_view rest(line);
        const std::string_view tag = nextWord(rest);

        if (tag == "v") {
            glm::vec3 value{};
            value.x = nextFloat(rest);
            value.y = nextFloat(rest);
            value.z = nextFloat(rest);
            positions.push_back(value);
        } else if (tag == "vt") {
            glm::vec2 value{};
            value.x = nextFloat(rest);
            value.y = nextFloat(rest);
            texCoords.push_back(value);
        } else if (tag == "vn") {
            glm::vec3 value{};
            value.x = nextFloat(rest);
            value.y = nextFloat(rest);
            value.z = nextFloat(rest);
            normals.push_back(value);
        } else if (tag == "mtllib") {
            const std::string_view libraryName = nextWord(rest);
            if (!libraryName.empty()) {
                result.baseColorTexture =
                    readBaseColorTexture(path, std::string(libraryName));
            }
        } else if (tag == "f") {
            // Вектор живёт снаружи цикла: иначе на каждой грани шло бы новое выделение.
            corners.clear();
            for (std::string_view field = nextWord(rest); !field.empty();
                 field = nextWord(rest)) {
                corners.push_back(parseCorner(field));
            }

            // Многоугольник режется веером от первого угла: OBJ допускает любое их число.
            for (size_t i = 2; i < corners.size(); ++i) {
                emit(corners[0]);
                emit(corners[i - 1]);
                emit(corners[i]);
            }
        }
    }

    if (result.mesh.vertices.empty()) {
        throw std::runtime_error("в модели нет ни одной грани: " + path);
    }
    return result;
}

}  // namespace obj
