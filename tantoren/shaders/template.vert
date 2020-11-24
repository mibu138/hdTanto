#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 outColor;

layout(set = 0, binding = 0) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 proj;
} matrices;

void main()
{
    gl_Position = matrices.proj * matrices.view * matrices.model * vec4(pos, 1.0);
    outColor = color;
}
