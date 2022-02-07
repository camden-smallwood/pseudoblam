#version 410 core

uniform vec3 camera_position;

struct light_data
{
    vec3 position;
    vec3 direction;
    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color;
};

uniform light_data light;

struct material_data
{
    sampler2D diffuse_texture;
    sampler2D specular_texture;
    float specular_amount;
    float specular_shininess;
    float ambient_amount;
};

uniform material_data material;

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec3 normal = normalize(frag_normal);

    // ambient lighting
    vec3 ambient = material.ambient_amount * light.ambient_color;

    // diffuse lighting
    vec3 light_direction = normalize(-light.direction);
    float diffuse_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diffuse_amount * light.diffuse_color * vec3(texture(material.diffuse_texture, frag_texcoord));

    // specular lighting
    vec3 camera_direction = normalize(camera_position - frag_position);
    vec3 camera_direction_reflected = reflect(-light_direction, normal);
    float specular_amount = pow(max(dot(camera_direction, camera_direction_reflected), 0.0), material.specular_shininess);
    vec3 specular = material.specular_amount * specular_amount * light.specular_color * vec3(texture(material.specular_texture, frag_texcoord));

    out_color = vec4(ambient + diffuse + specular, 1.0);
}
