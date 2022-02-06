#version 410 core

uniform vec3 camera_position;

uniform float ambient_amount;
uniform float specular_amount;
uniform float specular_shininess;

uniform vec3 light_position;
uniform vec3 light_color;

uniform sampler2D diffuse_texture;

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec3 normal = normalize(frag_normal);

    // ambient lighting
    vec3 ambient = ambient_amount * light_color;

    // diffuse lighting
    vec3 light_direction = normalize(light_position - frag_position);
    float diffuse_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diffuse_amount * light_color;

    // specular lighting
    vec3 camera_direction = normalize(camera_position - frag_position);
    vec3 camera_direction_reflected = reflect(-light_direction, normal);
    float specular_strength = pow(max(dot(camera_direction, camera_direction_reflected), 0.0), specular_shininess);
    vec3 specular = specular_amount * specular_strength * light_color;

    out_color = vec4(ambient + diffuse + specular, 1.0) * texture(diffuse_texture, frag_texcoord);
}
