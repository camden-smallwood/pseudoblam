/*
CAMERA.H
    Camera management declarations.
*/

#pragma once
#include <cglm/cglm.h>

/* ---------- types */

struct camera_data
{
    float look_sensitivity;
    
    float movement_speed;

    float horizontal_fov;
    float vertical_fov;

    float aspect_ratio;

    float near_clip;
    float far_clip;

    vec3 position;
    vec2 rotation;
    vec3 up;
    vec3 forward;

    mat4 view;
    mat4 projection;
};

/* ---------- prototypes/CAMERA.C */

void camera_initialize(struct camera_data *camera);
void camera_handle_screen_resize(struct camera_data *camera, int width, int height);
void camera_update(struct camera_data *camera, float delta_ticks);
