#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;

out vec4 vs_color;
out vec3 vs_normal;
out vec3 vs_worldpos;
out vec2 vs_tex_coord;

void main(void)
{
    vec4 pos = model_matrix * vec4(position, 1.0);
    gl_Position = projection_matrix * pos;

    vs_color = vec4(1.0, 1.0, 1.0, 1.0);
    vs_worldpos = (view_matrix * pos).xyz;
    vs_normal = normalize(mat3(model_matrix) * normal);
    vs_normal.y = -vs_normal.y;
    vs_tex_coord = vec2(0.0, 0.0);
}
