/*
CAMERA.C
    Camera management.
*/

#include <assert.h>
#include <math.h>
#include <string.h>

#include <cglm/cglm.h>

#include <SDL_keycode.h>

#include "input/input.h"
#include "camera.h"

/* ---------- public code */

void camera_initialize(struct camera_data *camera)
{
    assert(camera);

    camera->look_sensitivity = 5.0f;
    camera->movement_speed = 1.0f;
    camera->horizontal_fov = 90.0f;
    camera->vertical_fov = 0.0f;
    camera->aspect_ratio = 1.0f;
    camera->near_clip = 0.01f;
    camera->far_clip = 1000.0f;
    glm_vec3_copy((vec3){3, 2, 3}, camera->position);
    glm_vec3_copy((vec2){-130, -35}, camera->rotation);
    glm_vec3_copy((vec3){0, 1, 0}, camera->up);
}

void camera_handle_screen_resize(struct camera_data *camera, int width, int height)
{
    assert(camera);
    
    camera->aspect_ratio = (float)width / (float)height;

    float inverse_aspect_ratio = (float)height / (float)width;
    camera->vertical_fov = 2.0f * atanf(tanf(glm_rad(camera->horizontal_fov) / 2.0f) * inverse_aspect_ratio);
}

void camera_update(struct camera_data *camera, float delta_ticks)
{
    ivec2 mouse_motion_int;
    input_get_mouse_motion(&mouse_motion_int[0], &mouse_motion_int[1]);
    
    vec2 mouse_motion =
    {
        (float)mouse_motion_int[0],
        (float)-mouse_motion_int[1]
    };
    glm_vec2_scale(mouse_motion, 0.01f, mouse_motion);
    glm_vec2_scale(mouse_motion, camera->look_sensitivity, mouse_motion);
    glm_vec2_add(camera->rotation, mouse_motion, camera->rotation);

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
            sinf(pitch_radians),
            sinf(yaw_radians) * pitch_radians_cosine,
        },
        camera->forward);
    glm_vec3_normalize(camera->forward);

    glm_vec3_cross(camera->up, camera->forward, camera->right);
    glm_normalize(camera->right);

    vec3 up;
    glm_vec3_cross(camera->forward, camera->right, up);

    vec3 movement = {0.0f, 0.0f, 0.0f};
    int movement_inputs = 0;

    // Forwards and backwards camera movement
    if (input_is_key_down(SDL_SCANCODE_W) && !input_is_key_down(SDL_SCANCODE_S))
    {
        glm_vec3_add(movement, camera->forward, movement);
        movement_inputs++;
    }
    else if (input_is_key_down(SDL_SCANCODE_S) && !input_is_key_down(SDL_SCANCODE_W))
    {
        glm_vec3_sub(movement, camera->forward, movement);
        movement_inputs++;
    }

    // Horizontal camera movement
    if (input_is_key_down(SDL_SCANCODE_A) && !input_is_key_down(SDL_SCANCODE_D))
    {
        glm_vec3_add(movement, camera->right, movement);
        movement_inputs++;
    }
    else if (input_is_key_down(SDL_SCANCODE_D) && !input_is_key_down(SDL_SCANCODE_A))
    {
        glm_vec3_sub(movement, camera->right, movement);
        movement_inputs++;
    }

    // Vertical camera movement
    if (input_is_key_down(SDL_SCANCODE_R) && !input_is_key_down(SDL_SCANCODE_F))
    {
        glm_vec3_add(movement, up, movement);
        movement_inputs++;
    }
    else if (input_is_key_down(SDL_SCANCODE_F) && !input_is_key_down(SDL_SCANCODE_R))
    {
        glm_vec3_sub(movement, up, movement);
        movement_inputs++;
    }

    // Normalize the movement amount using the number of movement inputs
    glm_vec3_scale(movement, 1.0f / (movement_inputs ? movement_inputs : 1), movement);

    // Double movement amount if either shift key is down
    if (input_is_key_down(SDL_SCANCODE_LSHIFT) || input_is_key_down(SDL_SCANCODE_RSHIFT))
        glm_vec3_scale(movement, 2.0f, movement);
    
    // Scale the movement amount by the camera's movement speed per tick
    glm_vec3_scale(movement, camera->movement_speed * delta_ticks, movement);
    
    // Add the movement amount to the camera's position
    glm_vec3_add(camera->position, movement, camera->position);
    
    // Calculate the camera's target position
    vec3 camera_target;
    glm_vec3_add(camera->position, camera->forward, camera_target);

    glm_lookat(
        camera->position,
        camera_target,
        camera->up,
        camera->view);
    
    glm_perspective(
        camera->vertical_fov,
        camera->aspect_ratio,
        camera->near_clip,
        camera->far_clip,
        camera->projection);
}
