#version 330 core

layout (location = 0) out vec4 color;
in vec4 vs_normal;

uniform vec4 uni_color;

void main()
{
    vec4 temp = vec4((uni_color * (0.1 + abs(vs_normal.z)) + vec4(0.8, 0.9, 0.7, 1.0) * pow(abs(vs_normal.z), 40.0)).rgb, 1.0);
    color = uni_color * temp;
}
