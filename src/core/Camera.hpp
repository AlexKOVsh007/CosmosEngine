#pragma once

#include <glm/glm.hpp>

// Положение и направление взгляда. Ни Vulkan, ни оконной системы не знает.
class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 target);

    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }

    // Смещения в градусах; тангаж ограничен, иначе камера перевернётся.
    void rotate(float deltaYaw, float deltaPitch);

    // Шаг вдоль собственных осей: вперёд, вправо, вверх.
    void move(glm::vec3 localOffset);

    // Возврат туда, откуда камера начинала.
    void reset();

private:
    void updateFront();

    glm::vec3 position;
    glm::vec3 front;

    // Запоминаем начало, чтобы уметь вернуться.
    glm::vec3 startPosition;
    glm::vec3 startTarget;

    float yaw;
    float pitch;
};
