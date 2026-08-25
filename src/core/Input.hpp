#pragma once

struct GLFWwindow;

class Camera;
class Window;

// Клавиатура и мышь. Двигает камеру, но не владеет ею.
class Input {
public:
    explicit Input(Window& window);

    Input(const Input&) = delete;
    Input& operator=(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(Input&&) = delete;

    // Опрашивает клавиши и применяет накопленный поворот мыши.
    void apply(Camera& camera, float deltaSeconds);

private:
    // GLFW умеет звать только обычные функции: адрес объекта достаём из окна.
    static void onCursorMove(GLFWwindow* window, double x, double y);
    static void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void onFocusChange(GLFWwindow* window, int focused);

    void handleCursorMove(float x, float y);
    void setCaptured(bool value);

    // Доступ без владения.
    Window* window = nullptr;

    bool captured = false;
    bool hasLastPosition = false;
    float lastX = 0.0f;
    float lastY = 0.0f;

    // Накопленный поворот: мышь приходит колбэками, применяем раз в кадр.
    float pendingYaw = 0.0f;
    float pendingPitch = 0.0f;
};
