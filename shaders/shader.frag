#version 450

// sampler2D — image и правила его чтения в одном ресурсе.
layout(binding = 1) uniform sampler2D texSampler;

// Значения приходят уже смешанными: интерполяцию между вершинами делает железо.
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
