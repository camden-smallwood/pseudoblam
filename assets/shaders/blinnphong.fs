#version 410 core

uniform vec3 camera_position;

struct material_data
{
    sampler2D diffuse_texture;
    sampler2D specular_texture;
    float specular_amount;
    float specular_shininess;
    float ambient_amount;
};
uniform material_data material;

struct directional_light_data
{
    vec3 position;
    vec3 direction;
    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color;
};
#define MAXIMUM_DIRECTIONAL_LIGHTS 16
uniform uint directional_light_count;
uniform directional_light_data directional_lights[MAXIMUM_DIRECTIONAL_LIGHTS];

struct point_light_data
{
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color;
};
#define MAXIMUM_POINT_LIGHTS 16
uniform uint point_light_count;
uniform point_light_data point_lights[MAXIMUM_POINT_LIGHTS];

vec3 calculate_light(
    material_data material,
    vec2 frag_texcoord,
    vec3 normal,
    vec3 camera_direction,
    vec3 light_direction,
    vec3 diffuse_color,
    vec3 ambient_color,
    vec3 specular_color,
    float attenuation)
{
    vec3 diffuse_texture = vec3(texture(material.diffuse_texture, frag_texcoord));
    vec3 specular_texture = vec3(texture(material.specular_texture, frag_texcoord));

    // ambient shading
    vec3 ambient = material.ambient_amount * ambient_color * attenuation;

    // diffuse shading
    float diffuse_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diffuse_color * diffuse_amount * diffuse_texture * attenuation;

    // specular shading
    vec3 light_halfway_direction = normalize(light_direction + camera_direction);
    float specular_amount = pow(max(dot(normal, light_halfway_direction), 0.0), material.specular_shininess);
    vec3 specular = material.specular_amount * specular_color * specular_amount * specular_texture * attenuation;

    return ambient + diffuse + specular;
}

vec3 calculate_directional_light(
    directional_light_data light,
    material_data material,
    vec3 frag_position,
    vec2 frag_texcoord,
    vec3 normal,
    vec3 camera_direction)
{
    vec3 light_direction = normalize(-light.direction);

    return calculate_light(
        material,
        frag_texcoord,
        normal,
        camera_direction,
        light_direction,
        light.diffuse_color,
        light.ambient_color,
        light.specular_color,
        1.0);
}

vec3 calculate_point_light(
    point_light_data light,
    material_data material,
    vec3 frag_position,
    vec2 frag_texcoord,
    vec3 normal,
    vec3 camera_direction)
{
    vec3 light_direction = normalize(light.position - frag_position);
    float light_distance = length(light.position - frag_position);
    float light_attenuation = 1.0 / (light.constant + light.linear * light_distance + light.quadratic * (light_distance * light_distance));

    return calculate_light(
        material,
        frag_texcoord,
        normal,
        camera_direction,
        light_direction,
        light.diffuse_color,
        light.ambient_color,
        light.specular_color,
        light_attenuation);
}

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec3 normal = normalize(frag_normal);
    vec3 camera_direction = normalize(camera_position - frag_position);

    vec3 result = vec3(0.0);

    for (uint i = 0; i < directional_light_count; i++)
        result += calculate_directional_light(directional_lights[i], material, frag_position, frag_texcoord, normal, camera_direction);
    
    for (uint i = 0; i < point_light_count; i++)
        result += calculate_point_light(point_lights[i], material, frag_position, frag_texcoord, normal, camera_direction);

    out_color = vec4(result, 1.0);
}
