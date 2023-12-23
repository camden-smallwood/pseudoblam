/*
CAMERA.C
    Camera management.
*/

#include <assert.h>
#include <math.h>
#include <string.h>

#include <cglm/cglm.h>

#include <SDL_keycode.h>

#include "camera.h"

/* ---------- public code */

void camera_initialize(struct camera_data *camera)
{
    assert(camera);

    camera->field_of_view = 0.70f;
    camera->aspect_ratio = 1.0f;
    camera->near_clip = 0.01f;
    camera->far_clip = 1000.0f;
    glm_vec3_copy((vec3){0, 0, 0.7f}, camera->position);
    glm_vec2_copy((vec2){0, 0}, camera->rotation);
    glm_vec3_copy((vec3){0, 0, 1}, camera->up);
}

void camera_handle_screen_resize(struct camera_data *camera, int width, int height)
{
    assert(camera);
    
    camera->aspect_ratio = (float)width / (float)height;
}

void camera_rotate_towards_point(struct camera_data *camera, vec3 point, float amount)
{
    assert(camera);
    
    vec3 distance;
    glm_vec3_sub(point, camera->position, distance);
    
    float length = glm_vec3_norm(distance);
    
    camera->rotation[0] = glm_deg(atan2f(distance[1], distance[0])) * amount;
    camera->rotation[1] = glm_deg(asinf(distance[2] / length)) * amount;
}

void camera_update(struct camera_data *camera)
{
    // Clamp camera rotation pitch angle between -89 and 89 degrees to prevent flipping
    if (camera->rotation[1] > 89.0f)
        camera->rotation[1] = 89.0f;
    else if (camera->rotation[1] < -89.0f)
        camera->rotation[1] = -89.0f;

    float yaw_radians = glm_rad(camera->rotation[0]);
    float pitch_radians = glm_rad(camera->rotation[1]);
    float pitch_radians_cosine = cosf(pitch_radians);

    glm_vec3_copy(
        (vec3){
            cosf(yaw_radians) * pitch_radians_cosine,
            sinf(yaw_radians) * pitch_radians_cosine,
            sinf(pitch_radians),
        },
        camera->forward);
    glm_vec3_normalize(camera->forward);

    glm_vec3_cross(camera->up, camera->forward, camera->right);
    glm_normalize(camera->right);

    vec3 up;
    glm_vec3_cross(camera->forward, camera->right, up);

    // Add the movement amount to the camera's position
    glm_vec3_add(camera->position, camera->velocity, camera->position);
    
    // Calculate the camera's target position
    vec3 camera_target;
    glm_vec3_add(camera->position, camera->forward, camera_target);

    glm_lookat(
        camera->position,
        camera_target,
        camera->up,
        camera->view);
    
    glm_perspective(
        camera->field_of_view,
        camera->aspect_ratio,
        camera->near_clip,
        camera->far_clip,
        camera->projection);
}
