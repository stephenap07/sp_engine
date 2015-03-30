#version 330 core

layout (location = 0) out vec4 color;

in vec3 vs_normal;
in vec3 vs_worldpos;

uniform vec4 uni_color = vec4(0.0, 1.0, 1.0, 1.0);
uniform vec4 color_ambient = vec4(0.3, 0.3, 0.3, 1.0);
uniform vec4 color_diffuse = vec4(1.0, 1.0, 1.0, 1.0);
uniform vec4 color_specular = vec4(1.0, 1.0, 1.0, 1.0);
uniform float shininess = 97.0f;

uniform vec3 light_position = vec3(0.0, 3.0, 0.0);

void main()
{
   vec3 light_direction = normalize(light_position - vs_worldpos);
   vec3 normal = normalize(vs_normal);
   vec3 half_vector = normalize(light_direction + normalize(vs_worldpos));
   float diffuse = max(0.0, dot(normal, light_direction));
   float specular = pow(max(0.0, dot(vs_normal, half_vector)), shininess);

   color = uni_color * (color_ambient + diffuse * color_diffuse + specular * color_specular);
}
