#version 330 core

in vec2 vs_tex_coord;

layout (location = 0) out vec4 color;

uniform sampler2D tex;
uniform vec4 uni_color;

void main(void)
{
   color = vec4(0, 0, 0, 1); // vec4(1, 1, 1, texture(tex, vs_tex_coord).a) * uni_color;
}
