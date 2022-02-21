#pragma once

/* ---------- constants */

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
    int shader_index;
    int texture_count;
    struct material_texture *textures;
};

struct material_texture
{
    enum material_texture_usage usage;
    unsigned int id;
};

/* ---------- prototypes/MATERIALS.C */

const char *material_texture_usage_to_string(enum material_texture_usage usage);
