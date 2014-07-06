#version 330 core

in vec4 vs_color;
in vec3 vs_normal;
in vec2 vs_tex_coord;

layout (location = 0) out vec4 color;

uniform sampler2D tex;

void main(void)
{
   vec4 temp = vec4((vs_color * (0.1 + abs(vs_normal.z)) + vec4(0.8, 0.9, 0.7, 1.0) * pow(abs(vs_normal.z), 40.0)).rgb, 1.0);
   color = vs_color * texture(tex, vs_tex_coord) * temp;
}

