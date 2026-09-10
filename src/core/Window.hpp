#pragma once

#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

// Окно и цикл событий. О Vulkan не знает ничего.
class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    // Копия дала бы двух владельцев одного handle и двойное уничтожение.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Перемещать можно: handle переезжает, источник остаётся пустым.
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    bool shouldClose() const;
    void pollEvents() const;
    void show() const;

    // У свёрнутого окна нулевой размер: swapchain под него не создать.
    void waitWhileMinimized() const;

    // Отдаём наружу для создания surface: GLFW — не Vulkan, слой не нарушен.
    GLFWwindow* getNativeHandle() const { return handle; }

private:
    void centerOnPrimaryMonitor();

    GLFWwindow* handle = nullptr;
};
