#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec4 lightPosition;
    vec4 cameraPosition;
} ubo;

// sampler2D — image и правила его чтения в одном ресурсе.
layout(binding = 1) uniform sampler2D texSampler;

// Значения приходят уже смешанными: интерполяцию между вершинами делает железо.
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragWorldPosition;
layout(location = 2) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

const vec3 lightColor = vec3(1.0, 0.95, 0.85);
const float ambientStrength = 0.12;
const float specularStrength = 0.35;
const float shininess = 48.0;

void main() {
    const vec3 albedo = texture(texSampler, fragTexCoord).rgb;

    // Интерполяция между вершинами меняет длину вектора, а не только направление.
    const vec3 normal = normalize(fragNormal);
    const vec3 toLight = normalize(ubo.lightPosition.xyz - fragWorldPosition);
    const vec3 toCamera = normalize(ubo.cameraPosition.xyz - fragWorldPosition);

    // Чем ровнее свет падает на поверхность, тем ярче она освещена.
    const float diffuse = max(dot(normal, toLight), 0.0);

    // Блин-Фонг: биссектриса между светом и взглядом вместо отражённого луча.
    const vec3 halfway = normalize(toLight + toCamera);
    const float specular = pow(max(dot(normal, halfway), 0.0), shininess);

    const vec3 lit = albedo * (ambientStrength + diffuse * (1.0 - ambientStrength)) +
                     lightColor * specular * specularStrength;

    outColor = vec4(lit, 1.0);
}
