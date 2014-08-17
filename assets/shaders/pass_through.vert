#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;
uniform mat4 mv_matrix;

out vec4 vs_color;
out vec3 vs_normal;
out vec3 vs_worldpos;
out vec2 vs_tex_coord;

void main(void)
{
    vec4 pos = model_matrix * vec4(position, 1.0);
    gl_Position = projection_matrix * mv_matrix * vec4(position, 1.0);

    vs_worldpos = pos.xyz;
    vs_color = vec4(0.45, 0.75, 0.80, 1.0);
    vs_normal = normalize(mat3(model_matrix) * normal);

    vs_tex_coord = vec2(0.0, 0.0);
}
