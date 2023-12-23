/*
CAMERA.H
    Camera management declarations.
*/

#pragma once
#include <cglm/cglm.h>

/* ---------- types */

struct camera_data
{
    float field_of_view;
    float aspect_ratio;
    float near_clip;
    float far_clip;

    vec3 position;
    vec3 velocity;
    vec2 rotation;

    vec3 forward;
    vec3 right;
    vec3 up;

    mat4 view;
    mat4 projection;
};

/* ---------- prototypes/CAMERA.C */

void camera_initialize(struct camera_data *camera);
void camera_handle_screen_resize(struct camera_data *camera, int width, int height);
void camera_update(struct camera_data *camera);
void camera_rotate_towards_point(struct camera_data *camera, vec3 point, float amount);
