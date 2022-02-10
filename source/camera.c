#include <assert.h>
#include <string.h>
#include <SDL_keycode.h>
#include "camera.h"
#include "input.h"

/* ---------- public code */

void camera_initialize(struct camera_data *camera)
{
    assert(camera);

    camera->look_sensitivity = 5.0f;
    camera->movement_speed = 1.0f;
    camera->horizontal_fov = 74.0f;
    camera->vertical_fov = 0.0f;
    camera->aspect_ratio = 1.0f;
    camera->near_clip = 0.01f;
    camera->far_clip = 1000.0f;
    memcpy(camera->position, (vec3){3, 2, 3}, sizeof(vec3));
    memcpy(camera->rotation, (vec2){-130, -35}, sizeof(vec2));
    memcpy(camera->up, (vec3){0, 1, 0}, sizeof(vec3));
}

void camera_update(struct camera_data *camera, float delta_ticks)
{
    vec2 mouse_motion;
    input_get_mouse_motion(&mouse_motion[0], &mouse_motion[1]);
    
    mouse_motion[1] = -mouse_motion[1];
    glm_vec2_scale(mouse_motion, 100.0f * camera->look_sensitivity * delta_ticks, mouse_motion);
    glm_vec2_add(camera->rotation, mouse_motion, camera->rotation);

    // Clamp camera rotation pitch angle between -89 and 89 degrees to prevent flipping
    if (camera->rotation[1] > 89.0f)
        camera->rotation[1] = 89.0f;
    else if (camera->rotation[1] < -89.0f)
        camera->rotation[1] = -89.0f;

    float yaw_radians = glm_rad(camera->rotation[0]);
    float pitch_radians = glm_rad(camera->rotation[1]);
    float pitch_radians_cosine = cosf(pitch_radians);

    vec3 forward =
    {
        cosf(yaw_radians) * pitch_radians_cosine,
        sinf(pitch_radians),
        sinf(yaw_radians) * pitch_radians_cosine,
    };
    glm_vec3_normalize(forward);

    vec3 right;
    glm_vec3_cross(camera->up, forward, right);
    glm_normalize(right);

    vec3 up;
    glm_vec3_cross(forward, right, up);

    vec3 movement = {0.0f, 0.0f, 0.0f};

    // Forwards and backwards camera movement
    if (input_is_key_down(SDL_SCANCODE_W))
        glm_vec3_add(movement, forward, movement);
    else if (input_is_key_down(SDL_SCANCODE_S))
        glm_vec3_sub(movement, forward, movement);

    // Horizontal camera movement
    if (input_is_key_down(SDL_SCANCODE_A))
        glm_vec3_add(movement, right, movement);
    else if (input_is_key_down(SDL_SCANCODE_D))
        glm_vec3_sub(movement, right, movement);

    // Vertical camera movement
    if (input_is_key_down(SDL_SCANCODE_R))
        glm_vec3_add(movement, up, movement);
    else if (input_is_key_down(SDL_SCANCODE_F))
        glm_vec3_sub(movement, up, movement);

    // Double movement amount if either shift key is down
    if (input_is_key_down(SDL_SCANCODE_LSHIFT) || input_is_key_down(SDL_SCANCODE_RSHIFT))
        glm_vec3_scale(movement, 2.0f, movement);
    
    glm_vec3_scale(movement, camera->movement_speed * delta_ticks, movement);
    glm_vec3_add(camera->position, movement, camera->position);
    
    vec3 camera_target;
    glm_vec3_add(camera->position, forward, camera_target);

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
