#pragma once
#include <cglm/cglm.h>

/* ---------- constants */

enum material_flags
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

/* ---------- structures */

struct material_data
{
    char *name;

    enum material_shading_model shading_model;
    /* TODO: blend_func */

    unsigned int flags;
    int shader_index;

    int texture_count;
    struct material_texture *textures;

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

struct material_texture
{
    enum material_texture_usage usage;
    unsigned int id;
};

/* ---------- prototypes/MATERIALS.C */

const char *material_texture_usage_to_string(enum material_texture_usage usage);
