/*
MODEL_MATERIALS.H
    Model material management declarations.
*/

#pragma once
#include <cglm/cglm.h>

/* ---------- textures */

enum material_texture_usage
{
    _material_texture_usage_none,
    _material_texture_usage_diffuse,
    _material_texture_usage_specular,
    _material_texture_usage_ambient,
    _material_texture_usage_emissive,
    _material_texture_usage_height,
    _material_texture_usage_normals,
    _material_texture_usage_shininess,
    _material_texture_usage_opacity,
    _material_texture_usage_displacement,
    _material_texture_usage_lightmap,
    _material_texture_usage_reflection,
    _material_texture_usage_base_color,
    _material_texture_usage_normal_camera,
    _material_texture_usage_emission_color,
    _material_texture_usage_metalness,
    _material_texture_usage_diffuse_roughness,
    _material_texture_usage_ambient_occlusion,
    _material_texture_usage_unknown,
    _material_texture_usage_sheen,
    _material_texture_usage_clearcoat,
    _material_texture_usage_transmission,
    NUMBER_OF_MATERIAL_TEXTURE_USAGES
};

struct material_texture
{
    enum material_texture_usage usage;

    unsigned int id;
};

/* ---------- base properties */

enum material_base_flags
{
    _material_is_two_sided_bit,
    _material_enable_wireframe_bit,
    NUMBER_OF_MATERIAL_FLAGS
};

enum material_shading_model
{
    _material_shading_model_flat,
    _material_shading_model_gouraud,
    _material_shading_model_phong,
    _material_shading_model_blinn,
    _material_shading_model_toon,
    _material_shading_model_oren_nayar,
    _material_shading_model_minnaert,
    _material_shading_model_cook_torrance,
    _material_shading_model_no_shading,
    _material_shading_model_fresnel,
    _material_shading_model_pbr_brdf,
    NUMBER_OF_MATERIAL_SHADING_MODELS
};

enum material_blending_mode
{
    _material_blending_mode_default,
    _material_blending_mode_additive,
    NUMBER_OF_MATERIAL_BLENDING_MODES
};

struct material_base_properties
{
    char *name;

    unsigned int flags;

    enum material_shading_model shading_model;
    enum material_blending_mode blending_mode;

    float opacity;
    float transparency_factor;
    float bump_scaling;
    float shininess;
    float reflectivity;
    float shininess_strength;
    float refracti;

    vec3 color_diffuse;
    vec3 color_ambient;
    vec3 color_specular;
    vec3 color_emissive;
    vec3 color_transparent;
    vec3 color_reflective;

    char *global_background_image;
    char *global_shaderlang;

    char *shader_vertex;
    char *shader_fragment;
    char *shader_geo;
    char *shader_tesselation;
    char *shader_primitive;
    char *shader_compute;
};

/* ---------- pbr/brdf properties */

enum material_pbr_flags
{
    _material_use_pbr_base_color_texture_bit,
    _material_use_pbr_metalness_texture_bit,
    _material_use_pbr_diffuse_roughness_texture_bit,
    NUMBER_OF_MATERIAL_PBR_FLAGS
};

struct material_pbr_properties
{
    unsigned int flags;
    
    vec3 base_color;

    float metallic_factor;
    float roughness_factor;
    float anisotropy_factor;
};

/* ---------- specular/shininess properties */

struct material_specular_properties
{
    float specular_factor;
    float glossiness_factor;
};

/* ---------- sheen properties */

struct material_sheen_properties
{
    float color_factor;
    float roughness_factor;
};

/* ---------- clearcoat properties */

struct material_clearcoat_properties
{
    float clearcoat_factor;
    float roughness_factor;
};

/* ---------- transmission properties */

struct material_transmission_properties
{
    float transmission_factor;
};

/* ---------- volume properties */

struct material_volume_properties
{
    float thickness_factor;
    float attenuation_distance;
    vec3 attenuation_color;
};

/* ---------- emissive properties */

enum material_emissive_flags
{
    _material_use_emissive_texture_bit,
    NUMBER_OF_MATERIAL_EMISSIVE_FLAGS
};

struct material_emissive_properties
{
    unsigned int flags;
    float intensity;
};

/* ---------- ambient occlussion properties */

enum material_ambient_occlussion_flags
{
    _material_use_ambient_occlussion_texture_bit,
    NUMBER_OF_MATERIAL_AMBIENT_OCCLUSSION_FLAGS
};

struct material_ambient_occlussion_properties
{
    unsigned int flags;
};

/* ---------- */

struct material_data
{
    int shader_index;

    int texture_count;
    struct material_texture *textures;

    struct material_base_properties base_properties;
    struct material_pbr_properties pbr_properties;
    struct material_specular_properties specular_properties;
    struct material_sheen_properties sheen_properties;
    struct material_clearcoat_properties clearcoat_properties;
    struct material_transmission_properties transmission_properties;
    struct material_volume_properties volume_properties;
    struct material_emissive_properties emissive_properties;
    struct material_ambient_occlussion_properties ambient_occlussion_properties;
};

/* ---------- prototypes/MODEL_MATERIALS.C */

const char *material_texture_usage_to_string(enum material_texture_usage usage);
