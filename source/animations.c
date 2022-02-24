#include <cglm/cglm.h>
#include "animations.h"

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
