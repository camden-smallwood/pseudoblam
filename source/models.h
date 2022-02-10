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
    int texture_count;
    struct render_texture *textures;
};

struct render_texture
{
    unsigned int id;
};

struct render_mesh
{
    unsigned int vertex_array;
    unsigned int vertex_buffer;
    
    int vertex_count;
    struct render_vertex *vertices;

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
    vec3 tangent;
    vec3 bitangent;
};

/* ---------- prototypes/MODELS.C */

void render_model_load_file(struct render_model *model, const char *file_path);
