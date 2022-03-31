#version 410 core

#define _light_type_directional 0
#define _light_type_point 1
#define _light_type_spot 2

struct light_data
{
    uint type;
    vec3 position;
    vec3 direction;
    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color;
    float constant;
    float linear;
    float quadratic;
    float inner_cutoff;
    float outer_cutoff;
};
#define MAXIMUM_LIGHTS 16
uniform uint light_count;
uniform light_data lights[MAXIMUM_LIGHTS];

uniform vec3 camera_position;
uniform vec3 camera_direction;
uniform mat4 view;

uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D albedo_specular_texture;
uniform sampler2D material_texture;
uniform sampler2D emissive_texture;
uniform sampler2D ssao_texture;

in vec2 frag_texcoord;

layout(location = 0) out vec3 out_base_color;
layout(location = 1) out vec3 out_hdr_color;

void main()
{
    vec3 frag_position = texture(position_texture, frag_texcoord).rgb;
    vec3 frag_normal = texture(normal_texture, frag_texcoord).rgb;
    
    vec4 albedo_specular = texture(albedo_specular_texture, frag_texcoord);
    
    vec4 material = texture(material_texture, frag_texcoord);
    float material_ambient_amount = material.r;
    float material_specular_amount = material.g;
    float material_specular_shininess = material.b;

    vec3 emissive_color = texture(emissive_texture, frag_texcoord).rgb;

    float ambient_occlusion = texture(ssao_texture, frag_texcoord).r;

    vec3 light_color = (material_ambient_amount > 0.0 ? material_ambient_amount : 0.3) * albedo_specular.rgb * ambient_occlusion;

    for (uint i = 0; i < light_count; i++)
    {
        vec3 surface_direction;
        vec3 light_direction;
        
        if (lights[i].type == _light_type_directional)
        {
            surface_direction = camera_direction;
            light_direction = normalize(-lights[i].direction);
        }
        else
        {
            surface_direction = normalize(camera_position - frag_position);
            light_direction = normalize(lights[i].position - frag_position);
        }

        vec3 light_halfway_direction = normalize(light_direction + surface_direction);
        
        // diffuse
        float diffuse_amount = max(dot(light_direction, frag_normal), 0.0);
        vec3 diffuse_color = diffuse_amount * albedo_specular.rgb * lights[i].diffuse_color;

        // specular
        float specular_amount = 0.0;
        if (diffuse_amount > 0.0)
            specular_amount = pow(max(dot(frag_normal, light_halfway_direction), 0.0), material_specular_shininess);
        vec3 specular_color = material_specular_amount * ((specular_amount * lights[i].specular_color) * albedo_specular.a);

        if (lights[i].type != _light_type_directional)
        {
            float light_distance = length(lights[i].position - frag_position);
            float light_attenuation = 1.0 / (lights[i].constant + lights[i].linear * light_distance + lights[i].quadratic * (light_distance * light_distance));
            
            if (lights[i].type == _light_type_spot)
            {
                float light_theta = dot(light_direction, normalize(-lights[i].direction));
                float light_epsilon = lights[i].inner_cutoff - lights[i].outer_cutoff;
                float light_intensity = clamp((light_theta - lights[i].outer_cutoff) / light_epsilon, 0.0, 1.0);

                light_attenuation *= light_intensity;
            }

            diffuse_color *= light_attenuation;
            specular_color *= light_attenuation;
        }

        light_color += diffuse_color + specular_color;
    }
    
    light_color += emissive_color;

    out_base_color = light_color;

    float brightness = dot(light_color, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > 0.7)
        out_hdr_color = light_color;
    else
        out_hdr_color = vec3(0.0);
    
    out_hdr_color += emissive_color;
}
