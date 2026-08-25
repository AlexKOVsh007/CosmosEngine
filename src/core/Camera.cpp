#include "core/Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>

namespace {

// В нашей сцене вверх — это Z, а не Y.
constexpr glm::vec3 worldUp{0.0f, 0.0f, 1.0f};

constexpr float maxPitch = 89.0f;

}  // namespace

Camera::Camera(glm::vec3 position_, glm::vec3 target)
    : position(position_), startPosition(position_), startTarget(target) {
    reset();
}

void Camera::reset() {
    position = startPosition;
    front = glm::normalize(startTarget - startPosition);

    yaw = glm::degrees(std::atan2(front.y, front.x));
    pitch = glm::degrees(std::asin(front.z));
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, worldUp);
}

void Camera::rotate(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch = std::clamp(pitch + deltaPitch, -maxPitch, maxPitch);
    updateFront();
}

void Camera::move(glm::vec3 localOffset) {
    const glm::vec3 right = glm::normalize(glm::cross(front, worldUp));

    position += front * localOffset.x;
    position += right * localOffset.y;
    position += worldUp * localOffset.z;
}

void Camera::updateFront() {
    const float yawRadians = glm::radians(yaw);
    const float pitchRadians = glm::radians(pitch);

    front = glm::normalize(glm::vec3(std::cos(pitchRadians) * std::cos(yawRadians),
                                     std::cos(pitchRadians) * std::sin(yawRadians),
                                     std::sin(pitchRadians)));
}
