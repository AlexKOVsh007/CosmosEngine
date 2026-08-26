#include "core/Window.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

// Без колбэка GLFW молчит о своих ошибках: функции просто ничего не делают.
void reportGlfwError(int code, const char* description) {
    std::cerr << "GLFW error " << code << ": " << description << '\n';
}

}  // namespace

Window::Window(uint32_t width, uint32_t height, const std::string& title) {
    // Единственная функция GLFW, которую можно звать до инициализации.
    glfwSetErrorCallback(reportGlfwError);

    if (!glfwInit()) {
        throw std::runtime_error("glfwInit failed");
    }

    // GLFW родом из OpenGL и по умолчанию создаёт контекст OpenGL — запрещаем.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    handle = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                              title.c_str(), nullptr, nullptr);
    if (handle == nullptr) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow failed");
    }

    centerOnPrimaryMonitor();
}

// Иначе оконный менеджер ставит окно куда захочет.
void Window::centerOnPrimaryMonitor() {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (monitor == nullptr) {
        return;
    }
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (mode == nullptr) {
        return;
    }

    int width = 0;
    int height = 0;
    glfwGetWindowSize(handle, &width, &height);
    glfwSetWindowPos(handle, (mode->width - width) / 2, (mode->height - height) / 2);
}

Window::~Window() {
    // После перемещения остаётся пустая оболочка: ей гасить GLFW нельзя.
    if (handle == nullptr) {
        return;
    }
    glfwDestroyWindow(handle);
    glfwTerminate();
}

Window::Window(Window&& other) noexcept
    : handle(std::exchange(other.handle, nullptr)) {}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (handle != nullptr) {
            glfwDestroyWindow(handle);
            glfwTerminate();
        }
        handle = std::exchange(other.handle, nullptr);
    }
    return *this;
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(handle);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

void Window::waitWhileMinimized() const {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(handle, &width, &height);

    // waitEvents вместо pollEvents: спим до события, а не крутим цикл впустую.
    while (width == 0 || height == 0) {
        glfwWaitEvents();
        glfwGetFramebufferSize(handle, &width, &height);
    }
}
