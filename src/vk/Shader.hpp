#pragma once

#include <vulkan/vulkan.h>

#include <string>

class Context;

// Скомпилированный SPIR-V, загруженный в Vulkan.
class Shader {
public:
    Shader(const Context& context, std::string path);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    VkShaderModule getModule() const { return module; }
    const std::string& getPath() const { return path; }

private:
    void destroy() noexcept;

    // Доступ без владения.
    const Context* context = nullptr;

    std::string path;
    VkShaderModule module = VK_NULL_HANDLE;
};
