#version 330 core

layout (location = 0) out vec4 color;

uniform vec4 uni_color;

void main(void)
{
   color = uni_color; // vec4(1, 1, 1, texture(tex, vs_tex_coord).a) * uni_color;
}
