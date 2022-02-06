#version 410 core

uniform vec3 ambient_light_color;
uniform float ambient_light_intensity;

uniform vec3 diffuse_light_color;
uniform vec3 diffuse_light_position;

in vec3 frag_normal;

out vec4 out_color;

void main()
{
    vec3 ambient_color = ambient_light_color * ambient_light_intensity;
    out_color = vec4(ambient_color * diffuse_light_color, 1);
}
