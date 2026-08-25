#include "core/Input.hpp"

#include "core/Camera.hpp"
#include "core/Window.hpp"

#include <glm/glm.hpp>

namespace {

constexpr float moveSpeed = 2.5f;
constexpr float mouseSensitivity = 0.12f;

Input* inputFrom(GLFWwindow* window) {
    return static_cast<Input*>(glfwGetWindowUserPointer(window));
}

}  // namespace

Input::Input(Window& windowRef) : window(&windowRef) {
    GLFWwindow* handle = windowRef.getNativeHandle();

    // Записка при окне: по ней колбэки находят дорогу обратно к объекту.
    glfwSetWindowUserPointer(handle, this);

    glfwSetCursorPosCallback(handle, onCursorMove);
    glfwSetMouseButtonCallback(handle, onMouseButton);
    glfwSetKeyCallback(handle, onKey);
    glfwSetWindowFocusCallback(handle, onFocusChange);

    // Сырой ввод: смещения приходят от устройства, без ускорения системы.
    if (glfwRawMouseMotionSupported() != 0) {
        glfwSetInputMode(handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

void Input::onCursorMove(GLFWwindow* window, double x, double y) {
    if (Input* input = inputFrom(window)) {
        input->handleCursorMove(static_cast<float>(x), static_cast<float>(y));
    }
}

void Input::onMouseButton(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
        return;
    }
    if (Input* input = inputFrom(window)) {
        input->setCaptured(true);
    }
}

void Input::onKey(GLFWwindow* window, int key, int /*scancode*/, int action,
                  int /*mods*/) {
    if (action != GLFW_PRESS) {
        return;
    }
    Input* input = inputFrom(window);
    if (input == nullptr) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE) {
        input->setCaptured(false);
    }
}

// Иначе, переключившись из окна с захваченным курсором, мыши не вернуть.
void Input::onFocusChange(GLFWwindow* window, int focused) {
    if (focused != 0) {
        return;
    }
    if (Input* input = inputFrom(window)) {
        input->setCaptured(false);
    }
}

void Input::setCaptured(bool value) {
    captured = value;
    hasLastPosition = false;

    glfwSetInputMode(window->getNativeHandle(), GLFW_CURSOR,
                     value ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Input::handleCursorMove(float x, float y) {
    if (!captured) {
        return;
    }

    // Первый кадр после захвата даёт огромный скачок — пропускаем.
    if (!hasLastPosition) {
        lastX = x;
        lastY = y;
        hasLastPosition = true;
        return;
    }

    // Оба знака обратные: ось Y экрана растёт вниз, а угол рыскания — влево.
    pendingYaw -= (x - lastX) * mouseSensitivity;
    pendingPitch += (lastY - y) * mouseSensitivity;

    lastX = x;
    lastY = y;
}

void Input::apply(Camera& camera, float deltaSeconds) {
    camera.rotate(pendingYaw, pendingPitch);
    pendingYaw = 0.0f;
    pendingPitch = 0.0f;

    GLFWwindow* handle = window->getNativeHandle();
    const float step = moveSpeed * deltaSeconds;

    glm::vec3 offset{0.0f};
    if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) {
        offset.x += step;
    }
    if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) {
        offset.x -= step;
    }
    if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) {
        offset.y += step;
    }
    if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) {
        offset.y -= step;
    }
    if (glfwGetKey(handle, GLFW_KEY_SPACE) == GLFW_PRESS) {
        offset.z += step;
    }
    if (glfwGetKey(handle, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        offset.z -= step;
    }

    if (glfwGetKey(handle, GLFW_KEY_R) == GLFW_PRESS) {
        camera.reset();
        return;
    }

    camera.move(offset);
}
