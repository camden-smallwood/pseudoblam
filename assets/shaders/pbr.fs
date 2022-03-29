#version 410 core

uniform vec3 camera_position;

struct pbr_material_data
{
    vec3 albedo;
    float metallic;
    float roughness;
    float ambient_occlusion;
};
uniform pbr_material_data material;

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

in vec3 frag_position;
in vec3 frag_normal;
in vec3 frag_texcoord;

layout(location = 0) out vec4 out_color;

const float PI = 3.14159265359;

vec3 fresnel_schlick(float HdotV, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - HdotV, 0.0, 1.0), 5.0);
}

float distribution_ggx(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return num / denom;
}

float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometry_smith(float NdotL, float NdotV, float roughness)
{
    float ggx1 = geometry_schlick_ggx(NdotL, roughness):
    float ggx2 = geometry_schlick_ggx(NdotV, roughness);

    return ggx1 * ggx2;
}

void main()
{
    vec3 N = normalize(frag_normal);
    vec3 V = normalize(camera_position - frag_position);
    
    float NdotV = max(dot(N, V), 0.0);

    vec3 Lo = vec3(0.0);

    for (int i = 0; i < light_count; i++)
    {
        vec3 L = normalize(lights[i].position - frag_position);
        vec3 H = normalize(V + L);
        
        float NdotL = max(dot(N, L), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);

        float distance = length(lights[i].position - frag_position);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lights[i].diffuse_color * attenuation;
        
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, material.albedo, material.metallic);
        vec3 F = fresnel_schlick(HdotV, F0);

        float NDF = distribution_ggx(NdotH, material.roughness);
        float G = geometry_smith(NdotL, NdotV, material.roughness);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.metallic;
        
        Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * material.albedo * material.ambient_occlusion;

    // TODO:
    out_color = vec4(1, 1, 1, 1);
}
