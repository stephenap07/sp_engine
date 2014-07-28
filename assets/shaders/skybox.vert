#version 330 core

layout (location = 0) in vec3 in_position;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

out vec3 tex_coord;
uniform mat4 tc_rotate;

void main(void)
{
    gl_Position = projection_matrix * view_matrix * tc_rotate * vec4(in_position, 1.0);
    tex_coord = in_position;
}
