#version 410 core

uniform sampler2D shadow_texture;

uniform vec3 camera_position;

struct material_data
{
    vec3 diffuse_color;
    sampler2D diffuse_texture;
    
    vec3 specular_color;
    sampler2D specular_texture;
    float specular_amount;
    float specular_shininess;

    vec3 ambient_color;
    float ambient_amount;

    sampler2D emissive_texture;

    sampler2D normal_texture;
    float bump_scaling;
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

vec3 calculate_light(
    material_data material,
    vec2 frag_texcoord,
    vec3 normal,
    vec3 camera_direction,
    vec3 light_direction,
    vec3 light_diffuse_color,
    vec3 light_ambient_color,
    vec3 light_specular_color,
    float light_attenuation)
{
    float light_distance = length(light_direction);
    light_distance = light_distance * light_distance;

    vec3 diffuse_texture = vec3(texture(material.diffuse_texture, frag_texcoord));
    vec3 specular_texture = vec3(texture(material.specular_texture, frag_texcoord));

    // ambient shading
    vec3 ambient = material.ambient_amount * (material.ambient_color + light_ambient_color) * light_attenuation;

    // diffuse shading
    float diffuse_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diffuse_amount * (material.diffuse_color + light_diffuse_color) * diffuse_texture * light_attenuation;

    // specular shading
    vec3 light_halfway_direction = normalize(light_direction + camera_direction);
    float specular_amount = pow(max(dot(normal, light_halfway_direction), 0.0), material.specular_shininess);
    vec3 specular = (light_specular_color * specular_amount) * light_attenuation * (specular_texture * material.specular_amount);

    // emissive
    vec3 emissive = vec3(texture(material.emissive_texture, frag_texcoord));

    return ambient + diffuse + specular + emissive;
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

vec3 calculate_spot_light(
    spot_light_data light,
    material_data material,
    vec3 frag_position,
    vec2 frag_texcoord,
    vec3 normal,
    vec3 camera_direction)
{
    vec3 light_direction = normalize(light.position - frag_position);
    float light_distance = length(light.position - frag_position);
    float light_attenuation = 1.0 / (light.constant + light.linear * light_distance + light.quadratic * (light_distance * light_distance));
    float light_theta = dot(light_direction, normalize(-light.direction)); 
    float light_epsilon = light.inner_cutoff - light.outer_cutoff;
    float light_intensity = clamp((light_theta - light.outer_cutoff) / light_epsilon, 0.0, 1.0);

    return calculate_light(
        material,
        frag_texcoord,
        normal,
        camera_direction,
        light_direction,
        light.diffuse_color,
        light.ambient_color,
        light.specular_color,
        light_intensity * light_attenuation);
}

float calculate_shadow(
    vec3 frag_position,
    vec3 frag_normal,
    vec4 fragPosLightSpace,
    vec3 lightPos,
    vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadow_texture, projCoords.xy).r; 

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(frag_normal);
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF:
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadow_texture, 0);

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(shadow_texture, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;
    
    return 1.0 - shadow;
}

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_tbn;
in vec4 frag_position_light_space;

layout(location = 0) out vec4 out_base_color;
layout(location = 1) out vec4 out_hdr_color;

void main()
{
    vec3 normal = normalize(frag_tbn * (texture(material.normal_texture, frag_texcoord).rgb * 2.0 - 1.0) * material.bump_scaling);
    vec3 camera_direction = normalize(camera_position - frag_position);

    vec3 color = vec3(0.0);
    float shadow = 0.0;

    for (uint i = 0; i < directional_light_count; i++)
    {
        color += calculate_directional_light(directional_lights[i], material, frag_position, frag_texcoord, normal, camera_direction);
        shadow += calculate_shadow(frag_position, frag_normal, frag_position_light_space, directional_lights[i].position, directional_lights[i].direction);
    }
    
    for (uint i = 0; i < point_light_count; i++)
    {
        color += calculate_point_light(point_lights[i], material, frag_position, frag_texcoord, normal, camera_direction);
        shadow += calculate_shadow(frag_position, frag_normal, frag_position_light_space, point_lights[i].position, normalize(point_lights[i].position - frag_position));
    }
    
    for (uint i = 0; i < spot_light_count; i++)
    {
        color += calculate_spot_light(spot_lights[i], material, frag_position, frag_texcoord, normal, camera_direction);
        shadow += calculate_shadow(frag_position, frag_normal, frag_position_light_space, spot_lights[i].position, spot_lights[i].direction);
    }

    color *= shadow / (directional_light_count + point_light_count + spot_light_count);
    
    out_base_color = vec4(color, 1.0);

    float brightness = dot(color, vec3(0.2126, 0.5152, 0.0722));

    if (brightness > 1.0)
        out_hdr_color = vec4(color, 1.0);
    else
        out_hdr_color = vec4(0.0, 0.0, 0.0, 1.0);
}
