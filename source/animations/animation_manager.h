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
    _animation_state_paused_bit,
    _animation_state_fade_in_bit,
    _animation_state_fade_out_bit,
};

/* ---------- structures */

struct animation_node_state
{
    vec3 position;
    vec4 rotation;
    vec3 scale;
};

struct animation_state
{
    unsigned int flags;

    float time;
    float speed;
    float weight;

    float fade_in_duration;
    float fade_out_duration;

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

struct animation_state *animation_manager_get_animation_state(struct animation_manager *manager, int animation_index);

void animation_manager_set_animation_flags(struct animation_manager *manager, int animation_index, unsigned int flags);

bool animation_manager_is_animation_looping(struct animation_manager *manager, int animation_index);
void animation_manager_set_animation_looping(struct animation_manager *manager, int animation_index, bool looping);

bool animation_manager_is_animation_paused(struct animation_manager *manager, int animation_index);
void animation_manager_set_animation_paused(struct animation_manager *manager, int animation_index, bool paused);

bool animation_manager_is_animation_active(struct animation_manager *manager, int animation_index);
void animation_manager_set_animation_active(struct animation_manager *manager, int animation_index, bool active);

void animation_manager_set_animation_state_time(struct animation_manager *manager, int animation_index, float time);
void animation_manager_set_animation_state_speed(struct animation_manager *manager, int animation_index, float speed);

void animation_manager_set_animation_fade_in_duration(struct animation_manager *manager, int animation_index, float duration);
void animation_manager_set_animation_fade_out_duration(struct animation_manager *manager, int animation_index, float duration);

void animation_manager_update(struct animation_manager *manager, float delta_ticks);
