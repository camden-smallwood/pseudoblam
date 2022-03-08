#version 410 core

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

struct spot_light_data
{
    vec3 position;
    vec3 direction;
    float constant;
    float linear;
    float quadratic;
    float inner_cutoff;
    float outer_cutoff;
    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color;
};
#define MAXIMUM_SPOT_LIGHTS 16
uniform uint spot_light_count;
uniform spot_light_data spot_lights[MAXIMUM_SPOT_LIGHTS];

uniform vec3 camera_position;

uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D albedo_specular_texture;
uniform sampler2D material_texture;

in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec3 frag_position = texture(position_texture, frag_texcoord).rgb;
    vec3 frag_normal = texture(normal_texture, frag_texcoord).rgb;
    
    vec4 albedo_specular = texture(albedo_specular_texture, frag_texcoord);
    
    vec4 material = texture(material_texture, frag_texcoord);
    float material_specular_amount = material.r;
    float material_specular_shininess = material.g;

    vec3 light_color = albedo_specular.rgb * 0.1; // hard-coded ambient component
    vec3 camera_direction = normalize(camera_position - frag_position);

    for (uint i = 0; i < directional_light_count; i++)
    {
        // ambient
        vec3 ambient_color = directional_lights[i].ambient_color * albedo_specular.rgb;

        // diffuse
        vec3 light_direction = normalize(-directional_lights[i].direction);
        vec3 diffuse_color = max(dot(frag_normal, light_direction), 0.0) * albedo_specular.rgb * directional_lights[i].diffuse_color;

        // specular
        vec3 light_halfway_direction = normalize(light_direction + camera_direction);
        float specular_amount = pow(max(dot(frag_normal, light_halfway_direction), 0.0), material_specular_shininess);
        vec3 specular_color = directional_lights[i].specular_color * specular_amount * albedo_specular.a * material_specular_amount;

        light_color += ambient_color + diffuse_color + specular_color;
    }

    for (uint i = 0; i < point_light_count; i++)
    {
        // ambient
        vec3 ambient_color = point_lights[i].ambient_color * albedo_specular.rgb;

        // diffuse
        vec3 light_direction = normalize(point_lights[i].position - frag_position);
        vec3 diffuse_color = max(dot(frag_normal, light_direction), 0.0) * albedo_specular.rgb * point_lights[i].diffuse_color;

        // specular
        vec3 light_halfway_direction = normalize(light_direction + camera_direction);
        float specular_amount = pow(max(dot(frag_normal, light_halfway_direction), 0.0), material_specular_shininess);
        vec3 specular_color = point_lights[i].specular_color * specular_amount * albedo_specular.a * material_specular_amount;
        
        // attenuation
        float light_distance = length(point_lights[i].position - frag_position);
        float light_attenuation = 1.0 / (point_lights[i].constant + point_lights[i].linear * light_distance + point_lights[i].quadratic * (light_distance * light_distance));
        ambient_color *= light_attenuation;
        diffuse_color *= light_attenuation;
        specular_color *= light_attenuation;

        light_color += ambient_color + diffuse_color + specular_color;
    }

    for (uint i = 0; i < spot_light_count; i++)
    {
        // ambient
        vec3 ambient_color = spot_lights[i].ambient_color * albedo_specular.rgb;

        // diffuse
        vec3 light_direction = normalize(spot_lights[i].position - frag_position);
        vec3 diffuse_color = max(dot(frag_normal, light_direction), 0.0) * albedo_specular.rgb * spot_lights[i].diffuse_color;

        // specular
        vec3 light_halfway_direction = normalize(light_direction + camera_direction);
        float specular_amount = pow(max(dot(frag_normal, light_halfway_direction), 0.0), material_specular_shininess);
        vec3 specular_color = spot_lights[i].specular_color * specular_amount * albedo_specular.a * material_specular_amount;

        // spotlight intensity
        float light_theta = dot(light_direction, normalize(-spot_lights[i].direction)); 
        float light_epsilon = spot_lights[i].inner_cutoff - spot_lights[i].outer_cutoff;
        float light_intensity = clamp((light_theta - spot_lights[i].outer_cutoff) / light_epsilon, 0.0, 1.0);

        // attenuation
        float light_distance = length(spot_lights[i].position - frag_position);
        float light_attenuation = 1.0 / (spot_lights[i].constant + spot_lights[i].linear * light_distance + spot_lights[i].quadratic * (light_distance * light_distance));

        ambient_color *= light_attenuation * light_intensity;
        diffuse_color *= light_attenuation * light_intensity;
        specular_color *= light_attenuation * light_intensity;

        light_color += ambient_color + diffuse_color + specular_color;
    }

    out_color = vec4(light_color, 1.0);
}
