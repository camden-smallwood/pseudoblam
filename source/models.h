#pragma once
#include <cglm/cglm.h>
#include "materials.h"
#include "vertices.h"

/* ---------- types */

struct model_data
{
    int material_count;
    int bone_count;
    int mesh_count;
    struct material_data *materials;
    struct model_bone *bones;
    struct model_mesh *meshes;
};

struct model_bone
{
    int id;

    int parent_index;
    int first_child_index;
    int next_sibling_index;
    
    mat4 transform;
};

struct model_mesh
{
    unsigned int vertex_array;
    unsigned int vertex_buffer;
    
    enum vertex_type vertex_type;
    int vertex_count;
    void *vertex_data;

    int part_count;
    struct model_mesh_part *parts;
};

struct model_mesh_part
{
    int material_index;
    int vertex_index;
    int vertex_count;
};

struct model_iterator
{
    struct model_data *data;
    int index;
};

/* ---------- prototypes/MODELS.C */

void models_initialize(void);
void models_dispose(void);

int model_new(void);
void model_delete(int model_index);

struct model_data *model_get_data(int model_index);

void model_iterator_new(struct model_iterator *iterator);
int model_iterator_next(struct model_iterator *iterator);

int model_import_from_file(enum vertex_type vertex_type, const char *file_path);
