#version 330 core

uniform vec4 Ambient;

in vec4 Color;

out vec4 FragColor;

void main()
{
    vec4 scattered_light = Ambient;
    FragColor = min(Color * scattered_light, vec4(1.0));
}
