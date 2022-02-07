#pragma once
#include <cglm/cglm.h>

/* ---------- types */

struct render_model
{
    int material_count;
    int mesh_count;
    struct render_material *materials;
    struct render_mesh *meshes;
};

struct render_material
{
    unsigned int program;
};

struct render_mesh
{
    unsigned int vertex_array;
    unsigned int vertex_buffer;
    int part_count;
    struct render_mesh_part *parts;
};

struct render_mesh_part
{
    int material_index;
    int vertex_index;
    int vertex_count;
};

struct render_vertex
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

/* ---------- prototypes/MODELS.C */
