/*
ANIMATION_MANAGER.H
    Animation management declarations.
*/

#pragma once

#include <cglm/cglm.h>

/* ---------- constants */

enum animation_state_flags
{
    _animation_state_looping_bit,
};

/* ---------- structures */

struct animation_node_state
{
    vec3 position;
    vec4 rotation;
    vec3 scale;
    mat4 parent_transform;
    mat4 local_transform;
    mat4 global_transform;
    mat4 final_transform;
};

struct animation_state
{
    unsigned int flags;

    float time;
    float speed;

    struct animation_node_state *node_states;
};

struct animation_manager
{
    int model_index;

    int active_animation_count;
    unsigned int *active_animations_bit_vector;

    struct animation_state *states;
    
    mat4 *blended_node_matrices;
};

/* ---------- prototypes/ANIMATION_MANAGER.C */

void animation_manager_initialize(struct animation_manager *manager, int model_index);
void animation_manager_dispose(struct animation_manager *manager);
void animation_manager_update(struct animation_manager *manager, float delta_ticks);

void animation_manager_set_animation_flags(struct animation_manager *manager, int animation_index, unsigned int flags);

bool animation_manager_is_animation_active(struct animation_manager *manager, int animation_index);
void animation_manager_set_animation_active(struct animation_manager *manager, int animation_index, bool active);

void animation_manager_set_animation_state_time(struct animation_manager *manager, int animation_index, float time);
void animation_manager_set_animation_state_speed(struct animation_manager *manager, int animation_index, float speed);
