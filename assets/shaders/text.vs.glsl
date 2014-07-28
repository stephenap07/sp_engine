#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tex_coord;

out vec2 vs_tex_coord;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 uni_model;

void main(void)
{
   vs_tex_coord = tex_coord;
   gl_Position = uni_model * vec4(position.xy, 0.0, 1.0);
}
