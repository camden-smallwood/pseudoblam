#pragma once
#include <cglm/cglm.h>

/* ---------- structures */

struct animation_frame_position_data
{
    vec3 position;
    float time;
};

struct animation_frame_rotation_data
{
    vec3 rotation;
    float time;
};

struct animation_frame_scale_data
{
    vec3 scale;
    float time;
};

struct animation_bone_data
{
    int id;

    int position_frame_count;
    int rotation_frame_count;
    int scale_frame_count;

    struct animation_frame_position_data *position_frames;
    struct animation_frame_rotation_data *rotation_frames;
    struct animation_frame_scale_data *scale_frames;

    mat4 transform;
};

struct animation_data
{
    int id;
};

struct animation_graph_data
{
    int animation_count;
    struct animation_data *animations;
};

/* ---------- prototypes/ANIMATIONS.C */

void animations_initialize(void);
void animations_dispose(void);
void animations_update(float delta_ticks);

int animation_graph_new(void);
void animation_graph_delete(int animation_graph_index);

struct animation_graph_data *animation_graph_get_data(int animation_graph_index);
