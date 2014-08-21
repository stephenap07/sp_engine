#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 model_matrix;

void main(void)
{
   gl_Position = model_matrix * vec4(position.xy, 0.0f, 1.0);
}
