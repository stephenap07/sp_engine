#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coord;
layout (location = 3) in vec2 tangent;
layout (location = 4) in vec4 blend_index;
layout (location = 5) in vec4 blend_weight;

out vec4 vs_color;
out vec3 vs_normal;
out vec2 vs_tex_coord;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

uniform mat4 model_matrix;
uniform mat4 bone_matrices[80];

void main(void)
{
    vs_color = vec4(1.0, 1.0, 1.0, 1.0);
    vs_normal = normalize(normal);
    vs_tex_coord = tex_coord;

    mat4 m = mat4(1.0);
    /*
    m = bone_matrices[int(blend_index.x)] * blend_weight.x;
    m += bone_matrices[int(blend_index.y)] * blend_weight.y;
    m += bone_matrices[int(blend_index.z)] * blend_weight.z;
    m += bone_matrices[int(blend_index.w)] * blend_weight.w;
    */

    gl_Position = projection_matrix * view_matrix * model_matrix * m * vec4(position, 1.0);
}
