/*
MODELS.H
    3D model management declarations.
*/

#pragma once
#include <stdbool.h>
#include <cglm/cglm.h>
#include "models/materials.h"
#include "render/vertices.h"

/* ---------- types */

struct model_data
{
    int material_count;
    int node_count;
    int mesh_count;
    int animation_count;
    struct material_data *materials;
    struct model_node *nodes;
    struct model_mesh *meshes;
    struct model_animation *animations;
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
    unsigned int element_buffer;
};

struct model_mesh_part
{
    int material_index;
    int index_start;
    int index_count;
};

struct model_animation
{
    char *name;

    float duration;
    float ticks_per_second;

    int channel_count;
    struct model_animation_channel *channels;
};

enum model_animation_channel_type
{
    _model_animation_channel_type_node,
    _model_animation_channel_type_mesh,
    _model_animation_channel_type_morph,
    NUMBER_OF_MODEL_ANIMATION_CHANNEL_TYPES
};

struct model_animation_channel
{
    enum model_animation_channel_type type;
    union
    {
        int node_index;
        int mesh_index;
    };
    int position_key_count;
    int rotation_key_count;
    int scaling_key_count;
    int mesh_key_count;
    int morph_key_count;
    struct model_animation_position_key *position_keys;
    struct model_animation_rotation_key *rotation_keys;
    struct model_animation_scaling_key *scaling_keys;
    struct model_animation_mesh_key *mesh_keys;
    struct model_animation_morph_key *morph_keys;
};

struct model_animation_position_key
{
    float time;
    vec3 position;
};

struct model_animation_rotation_key
{
    float time;
    vec4 rotation;
};

struct model_animation_scaling_key
{
    float time;
    vec3 scaling;
};

struct model_animation_mesh_key
{
    float time;
    int mesh_index;
};

struct model_animation_morph_key
{
    float time;
    int count;
    int *values;
    float *weights;
};

struct model_animation_state
{
    float time;
    mat4 *node_matrices;
};

struct model_animation_manager
{
    int model_index;
    unsigned int *active_animations_bit_vector;
    struct model_animation_state *animation_states;
};

/* ---------- prototypes/MODELS.C */

void models_initialize(void);
void models_dispose(void);

int model_new(void);
void model_delete(int model_index);

struct model_data *model_get_data(int model_index);

void model_iterator_new(struct model_iterator *iterator);
int model_iterator_next(struct model_iterator *iterator);

int model_find_root_node(struct model_data *model);
int model_find_node_by_name(struct model_data *model, const char *node_name);
int model_node_add_child_node(struct model_data *model, int node_index, struct model_node *child_node);

/* ---------- prototypes/MODEL_IMPORT.C */

int model_import_from_file(enum vertex_type vertex_type, const char *file_path);

/* ---------- prototypes/MODEL_ANIMATION.C */

void model_animations_initialize(struct model_animation_manager *manager, int model_index);
void model_animations_dispose(struct model_animation_manager *manager);
void model_set_animation_active(struct model_animation_manager *manager, int animation_index, bool active);
void model_animations_update(struct model_animation_manager *manager, float delta_ticks);
