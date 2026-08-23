#include "vk/Shader.hpp"

#include "vk/Context.hpp"

#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

std::vector<char> readBinaryFile(const std::string& path) {
    // ate — открыть с конца, чтобы сразу узнать размер файла.
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("не удалось открыть " + path);
    }

    const std::streamsize size = file.tellg();
    std::vector<char> bytes(static_cast<size_t>(size));

    file.seekg(0);
    file.read(bytes.data(), size);
    return bytes;
}

}  // namespace

Shader::Shader(const Context& ctx, std::string shaderPath)
    : context(&ctx), path(std::move(shaderPath)) {
    const std::vector<char> code = readBinaryFile(path);

    const VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        // SPIR-V состоит из 32-битных слов; вектор выровнен достаточно.
        .pCode = reinterpret_cast<const uint32_t*>(code.data()),
    };

    if (vkCreateShaderModule(ctx.getDevice(), &info, nullptr, &module) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateShaderModule failed: " + path);
    }
}

Shader::~Shader() {
    destroy();
}

void Shader::destroy() noexcept {
    if (module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(context->getDevice(), module, nullptr);
        module = VK_NULL_HANDLE;
    }
}

Shader::Shader(Shader&& other) noexcept
    : context(other.context),
      path(std::move(other.path)),
      module(std::exchange(other.module, VK_NULL_HANDLE)) {}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        destroy();
        context = other.context;
        path = std::move(other.path);
        module = std::exchange(other.module, VK_NULL_HANDLE);
    }
    return *this;
}
