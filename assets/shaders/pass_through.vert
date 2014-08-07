#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;
out vec3 vs_normal;
out vec3 vs_position;

void main(void)
{
    vs_normal = normal;
    vs_position = position;

    gl_Position = projection_matrix * model_matrix * vec4(position, 1.0);
}
