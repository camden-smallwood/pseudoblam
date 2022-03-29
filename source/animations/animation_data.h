/*
ANIMATION_DATA.H
    Animation data types and declarations.
*/

#pragma once

struct animation_data
{
    char *name;

    float duration;
    float ticks_per_second;

    int channel_count;
    struct animation_channel *channels;
};

enum animation_channel_type
{
    _animation_channel_type_node,
    _animation_channel_type_mesh,
    _animation_channel_type_morph,
    NUMBER_OF_ANIMATION_CHANNEL_TYPES
};

struct animation_channel
{
    enum animation_channel_type type;
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
    struct animation_position_key *position_keys;
    struct animation_rotation_key *rotation_keys;
    struct animation_scaling_key *scaling_keys;
    struct animation_mesh_key *mesh_keys;
    struct animation_morph_key *morph_keys;
};

struct animation_position_key
{
    float time;
    vec3 position;
};

struct animation_rotation_key
{
    float time;
    vec4 rotation;
};

struct animation_scaling_key
{
    float time;
    vec3 scaling;
};

struct animation_mesh_key
{
    float time;
    int mesh_index;
};

struct animation_morph_key
{
    float time;
    int count;
    int *values;
    float *weights;
};
