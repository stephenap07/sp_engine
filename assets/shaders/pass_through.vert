#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;

out vec4 vs_normal;

void main(void)
{
    float scale = 1.0f;
    vs_normal = normalize(view_matrix * model_matrix * vec4(normal, scale));
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, scale);
}
