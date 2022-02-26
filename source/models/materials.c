#include <stdio.h>
#include <stdlib.h>

#include <assimp/material.h>

#include "models/materials.h"

/* ---------- public code */

const char *material_texture_usage_to_string(
    enum material_texture_usage usage)
{
    switch (usage)
    {
    case _material_texture_usage_none:
        return "none";
    case _material_texture_usage_diffuse:
        return "diffuse";
    case _material_texture_usage_specular:
        return "specular";
    case _material_texture_usage_ambient:
        return "ambient";
    case _material_texture_usage_emissive:
        return "emissive";
    case _material_texture_usage_height:
        return "height";
    case _material_texture_usage_normals:
        return "normals";
    case _material_texture_usage_shininess:
        return "shininess";
    case _material_texture_usage_opacity:
        return "opacity";
    case _material_texture_usage_displacement:
        return "displacement";
    case _material_texture_usage_lightmap:
        return "lightmap";
    case _material_texture_usage_reflection:
        return "reflection";
    case _material_texture_usage_base_color:
        return "base color";
    case _material_texture_usage_normal_camera:
        return "normal camera";
    case _material_texture_usage_emission_color:
        return "emission color";
    case _material_texture_usage_metalness:
        return "metalness";
    case _material_texture_usage_diffuse_roughness:
        return "diffuse roughness";
    case _material_texture_usage_ambient_occlusion:
        return "ambient occlusion";
    case _material_texture_usage_unknown:
        return "unknown";
    case _material_texture_usage_sheen:
        return "sheen";
    case _material_texture_usage_clearcoat:
        return "clearcoat";
    case _material_texture_usage_transmission:
        return "transmission";
    default:
        fprintf(stderr, "ERROR: unhandled texture usage %i\n", usage);
        exit(EXIT_FAILURE);
    }
}
