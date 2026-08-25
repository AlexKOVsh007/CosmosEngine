#version 450

// Номер binding совпадает с тем, что записано в наборе дескрипторов.
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

// Номера location совпадают с описанием атрибутов на стороне C++.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;

void main() {
    // Читается справа налево: поставить в мир, посмотреть из камеры, спроецировать.
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
}
