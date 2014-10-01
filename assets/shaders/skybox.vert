#version 330 core

layout (location = 0) in vec3 in_position;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

out vec3 tex_coord;
uniform mat4 rotate_matrix;

void main(void)
{
    gl_Position = projection_matrix * view_matrix * rotate_matrix * vec4(in_position, 1.0);
    tex_coord = in_position;
}
