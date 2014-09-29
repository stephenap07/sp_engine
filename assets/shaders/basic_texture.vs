#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;

out vec4 vs_color;
out vec3 vs_normal;
out vec2 vs_tex_coord;
out vec3 vs_worldpos;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;

void main(void)
{
   vs_color = vec4(1.0, 1.0, 1.0, 1.0);
   vs_normal = -vec3(0.0, 1.0, 0.0);
   vs_tex_coord = tex_coord;

   vec4 pos = model_matrix * vec4(position, 1.0);
   vs_worldpos = pos.xyz;
   gl_Position = projection_matrix * view_matrix * pos;
}
