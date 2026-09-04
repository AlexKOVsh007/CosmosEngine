#version 450

// Номер binding совпадает с тем, что записано в наборе дескрипторов.
layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 lightPosition;
    vec4 cameraPosition;
} ubo;

// Номера location совпадают с описанием атрибутов на стороне C++.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec3 fragWorldPosition;
layout(location = 2) out vec3 fragNormal;

void main() {
    const vec4 worldPosition = ubo.model * vec4(inPosition, 1.0);

    // Читается справа налево: поставить в мир, посмотреть из камеры, спроецировать.
    gl_Position = ubo.projection * ubo.view * worldPosition;

    fragTexCoord = inTexCoord;
    fragWorldPosition = worldPosition.xyz;

    // Транспонированная обратная: обычная сломала бы нормаль при масштабировании.
    fragNormal = mat3(transpose(inverse(ubo.model))) * inNormal;
}
