#version 330 core

in vec2 vs_tex_coord;

layout (location = 0) out vec4 color;

uniform sampler2D tex;
uniform vec4 uni_color;

void main(void)
{
   color = texture(tex, vs_tex_coord).rrrr * uni_color;
}
