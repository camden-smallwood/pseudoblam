/*
MODELS.H
    3D model management declarations.
*/

#pragma once

#include <stdbool.h>

#include <cglm/cglm.h>

#include "models/model_materials.h"
#include "rasterizer/rasterizer_vertices.h"
#include "animations/animation_data.h"

/* ---------- constants */

enum
{
    MAXIMUM_NUMBER_OF_MODEL_NODES = 256,
};

/* ---------- types */

struct model_data
{
    int material_count;
    int node_count;
    int marker_count;
    int mesh_count;
    int animation_count;
    struct material_data *materials;
    struct model_node *nodes;
    struct model_marker *markers;
    struct model_mesh *meshes;
    struct animation_data *animations;
};

struct model_iterator
{
    struct model_data *data;
    int index;
};

struct model_node
{
    char *name;

    int parent_index;
    int first_child_index;
    int next_sibling_index;
    
    mat4 offset_matrix;
    mat4 transform;
};

struct model_marker
{
    char *name;

    int node_index;

    vec3 position;
    vec3 rotation;
};

struct model_mesh
{
    enum vertex_type vertex_type;
    int vertex_count;
    void *vertex_data;

    int index_count;
    int *indices;

    int part_count;
    struct model_mesh_part *parts;
    
    unsigned int vertex_array;
    unsigned int vertex_buffer;
    unsigned int index_buffer;
    unsigned int uniform_buffer;
};

struct model_mesh_part
{
    int material_index;

    int vertex_start;
    int vertex_count;

    int index_start;
    int index_count;
};

/* ---------- prototypes/MODELS.C */

void models_initialize(void);
void models_dispose(void);

int model_new(void);
void model_delete(int model_index);

struct model_data *model_get_data(int model_index);

void model_iterator_new(struct model_iterator *iterator);
int model_iterator_next(struct model_iterator *iterator);

int model_get_root_node(struct model_data *model);
int model_find_node_by_name(struct model_data *model, const char *node_name);

int model_node_add_child_node(struct model_data *model, int node_index, struct model_node *child_node);

int model_find_marker_by_name(struct model_data *model, const char *marker_name);

int model_find_animation_by_name(int model_index, const char *animation_name);

/* ---------- prototypes/MODEL_IMPORT.C */

int model_import_from_file(enum vertex_type vertex_type, const char *file_path);
