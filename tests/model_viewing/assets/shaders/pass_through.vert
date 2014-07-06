#version 330 core

layout(location = 0) in vec3 position;

//out vec4 vs_color;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;

void main(void)
{
   //vs_color = vec4(0.0, 0.0, 1.0, 1.0);
   gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}
