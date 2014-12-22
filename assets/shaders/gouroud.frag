#version 330 core

in vec4 vs_color;
in vec3 vs_normal;
in vec3 vs_worldpos;
in vec2 vs_tex_coord;

layout (location = 0) out vec4 color;

uniform sampler2D tex;

uniform bool is_textured = false;
uniform vec4 color_ambient = vec4(0.2, 0.2, 0.2, 1.0);
uniform vec4 color_light = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 color_specular = vec4(1.0, 1.0, 1.0, 1.0);
uniform float shininess = 13.0f;

uniform vec3 light_position = vec3(0.0, 165.0, 0.0);

void main(void)
{
    vec3 light_direction = normalize(vs_worldpos - light_position);
    vec3 normal = normalize(vs_normal);

    vec3 half_vector = normalize(light_direction + normalize(vs_worldpos));
    float diffuse = max(0.0, dot(normal, light_direction));
    float specular = max(0.0, dot(normal, half_vector));

    if (diffuse == 0.0) {
        specular = 0.0;
    } else {
        specular = pow(specular, shininess);
    }

    vec4 tex_color;
    if (is_textured) {
        tex_color = texture(tex, vs_tex_coord);
    } else {
        tex_color = vs_color;
    }

    vec4 scattered_light = color_ambient + color_light * diffuse;
    vec4 reflected_light = color_light * specular;

    vec3 rgb = min(tex_color * (scattered_light + reflected_light), vec4(1.0)).rgb;
    color = vec4(rgb, tex_color.a);
}
