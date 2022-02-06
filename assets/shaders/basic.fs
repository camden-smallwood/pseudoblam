#version 410 core

uniform sampler2D diffuse_texture;

uniform float ambient_amount;

uniform vec3 light_position;
uniform vec3 light_color;

in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec3 normal = normalize(frag_normal);

    vec3 ambient = ambient_amount * light_color;

    vec3 light_direction = normalize(light_position);
    float diffuse_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diffuse_amount * light_color;

    out_color = vec4(ambient + diffuse, 1.0) * texture(diffuse_texture, frag_texcoord);
}
