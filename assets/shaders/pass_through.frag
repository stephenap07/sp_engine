#version 330 core

layout (location = 0) out vec4 color;

in vec3 vs_normal;
in vec3 vs_position;

uniform mat4 model_matrix;
uniform vec4 uni_color;

layout(std140) uniform globalMatrices {
    mat4 projection_matrix;
    mat4 view_matrix;
};

float cosTh;
float cosTi;
vec3 normal;
vec3 light_p;

void main()
{
    mat4 normal_mat = transpose(inverse(model_matrix));
    normal = normalize(vec3(normal_mat * vec4(vs_normal, 1.0)));
    light_p = vec3(view_matrix * vec4(0.0, 3.0, 0.0, 1.0));

    vec3 pv = vec3(projection_matrix * model_matrix * vec4(vs_position, 1.0));
    vec3 v = normalize(pv - vs_position);
    vec3 h = normalize(v + light_p);
    vec3 n = normalize(vec3(model_matrix * vec4(normal, 1.0)));
    cosTh = max(dot(n, h), 0);
    cosTi = max(dot(n, light_p), 0);

    float m = 0.1;
    vec3 Kd = vec3(uni_color) / 3.14159;
    //vec3 Ks = (m + 8) / (8 * 3.14159);
    vec3 Lo = (Kd + 3.0f * pow(cosTh, m)) * light_p * cosTi;

    color = vec4(Lo, 1.0f);
}
