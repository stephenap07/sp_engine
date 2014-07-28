#version 330 core

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 tex_coord;

out vec2 vs_tex_coord;
uniform mat4 model_matrix;

void main(void)
{
   vs_tex_coord = tex_coord;
   gl_Position = vec4(position.xyz, 1.0);
}
