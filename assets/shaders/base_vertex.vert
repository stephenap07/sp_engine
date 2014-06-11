#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

out vec4 vs_color;
out vec3 vs_normal;
out vec2 vs_tex_coord;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;

void main(void)
{
   vs_color = vec4(1.0, 1.0, 1.0, 1.0);
   vs_normal = normalize(normal);
   vs_tex_coord = tex_coord;

   gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}

