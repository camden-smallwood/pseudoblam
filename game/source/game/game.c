/*
GAME.C
    Game management code.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "common/common.h"
#include "camera/camera.h"
#include "game/game.h"
#include "objects/objects.h"
#include "objects/lights.h"

/* ---------- private constants */

enum game_flags
{
    _game_input_tab_bit,
    
    _game_input_h_bit,

    _game_input_1_bit,
    _game_input_2_bit,
    _game_input_3_bit,
    _game_input_4_bit,
    _game_input_5_bit,
    _game_input_6_bit,
    _game_input_7_bit,
    _game_input_8_bit,
    _game_input_9_bit,
    _game_input_0_bit,

    _game_played_initial_ready_animation_bit,
};

/* ---------- private variables */

struct
{
    unsigned int flags;
    
    struct camera_data camera;
    float camera_look_sensitivity;
    float camera_movement_speed;

    int plane_object_index;
    int flashlight_light_index;
    int weapon_object_index;
    int grunt_object_index;
} game_globals;

/* ---------- private prototypes */

static void game_update_camera(float delta_ticks);
static void game_update_objects(void);

/* ---------- public code */

struct camera_data *game_get_player_camera(void)
{
    return &game_globals.camera;
}

void game_initialize(void)
{
    memset(&game_globals, 0, sizeof(game_globals));
    
    // TODO
}

void game_dispose(void)
{
    // TODO
}

void game_handle_screen_resize(int width, int height)
{
    camera_handle_screen_resize(&game_globals.camera, width, height);
}

void game_load_content(void)
{
    // Initialize ground plane
    game_globals.plane_object_index = object_new();
    struct object_data *plane_object = object_get_data(game_globals.plane_object_index);
    plane_object->model_index = model_import_from_file(_vertex_type_rigid, "../assets/models/plane.fbx");
    object_initialize(game_globals.plane_object_index);

    // Initialize grunt character
    game_globals.grunt_object_index = object_new();
    struct object_data *grunt = object_get_data(game_globals.grunt_object_index);
    glm_vec3_copy((vec3){-5, 0, 0}, grunt->position);
    glm_vec3_copy((vec3){0.1f, 0.1f, 0.1f}, grunt->scale);
    grunt->model_index = model_import_from_file(_vertex_type_skinned, "../assets/models/grunt.fbx");
    object_initialize(game_globals.grunt_object_index);
    animation_manager_set_animation_looping(&grunt->animations, 0, true);

    // Initialize first person weapons
    game_globals.weapon_object_index = object_new();
    struct object_data *weapon = object_get_data(game_globals.weapon_object_index);
    glm_vec3_copy((vec3){0.01f, 0.01f, 0.01f}, weapon->scale);
    weapon->model_index = model_import_from_file(_vertex_type_skinned, "../assets/models/assault_rifle.fbx");
    object_initialize(game_globals.weapon_object_index);

    int moving_animation_index = model_find_animation_by_name(weapon->model_index, "first_person moving");
    animation_manager_set_animation_looping(&weapon->animations, moving_animation_index, true);
    
    // Initialize the player camera
    camera_initialize(&game_globals.camera);
    game_globals.camera_look_sensitivity = 5.0f;
    game_globals.camera_movement_speed = 1.0f;

    // Create scene lights
    struct light_data *light;

    // Initialize the flashlight
    game_globals.flashlight_light_index = light_new();
    light = light_get_data(game_globals.flashlight_light_index);
    light->type = _light_type_spot;
    SET_BIT(light->flags, _light_is_hidden_bit, true);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.1f;
    light->quadratic = 0.32f;
    light->inner_cutoff = 12.5f;
    light->outer_cutoff = 17.5f;

    // Initialize a point light for the scene
    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){-10.0f, -10.0f, 10.0f}, light->position);
    glm_vec3_copy((vec3){0.4f, 0.6f, 0.8f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.009f;
    light->quadratic = 0.0032f;

    // Initialize a point light for the scene
    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){10.0f, -10.0f, 10.0f}, light->position);
    glm_vec3_copy((vec3){0.8f, 0.6f, 0.4f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.009f;
    light->quadratic = 0.0032f;
}

void game_update(float delta_ticks)
{
    game_update_camera(delta_ticks);
    game_update_objects();
}

/* ---------- private code */

static void game_update_camera(float delta_ticks)
{
    // Rotate the camera towards the grunt
    {
        static float rotation_amount = 0.0f;

        if (rotation_amount < 1.0f)
        {
            struct object_data *grunt = object_get_data(game_globals.grunt_object_index);
            camera_rotate_towards_point(&game_globals.camera, grunt->position, rotation_amount);
            
            rotation_amount = fminf(1.0f, rotation_amount + (delta_ticks * 2.0f));
        }
    }

    const uint8_t *keys = SDL_GetKeyboardState(NULL);

    // Cycle through camera movement speeds
    if (keys[SDL_SCANCODE_TAB])
    {
        SET_BIT(game_globals.flags, _game_input_tab_bit, true);
    }
    else if (TEST_BIT(game_globals.flags, _game_input_tab_bit))
    {
        SET_BIT(game_globals.flags, _game_input_tab_bit, false);
        
        if (game_globals.camera_movement_speed < 1000.0f)
            game_globals.camera_movement_speed *= 2.0f;
        else
            game_globals.camera_movement_speed = 1.0f;
    }

    int mouse_motion_x, mouse_motion_y;
    SDL_GetRelativeMouseState(&mouse_motion_x, &mouse_motion_y);
    
    vec2 mouse_motion = {(float)-mouse_motion_x, (float)-mouse_motion_y};

    glm_vec2_scale(mouse_motion, 0.01f, mouse_motion);
    glm_vec2_scale(mouse_motion, game_globals.camera_look_sensitivity, mouse_motion);
    glm_vec2_add(game_globals.camera.rotation, mouse_motion, game_globals.camera.rotation);

    vec3 forward_movement = GLM_VEC3_ZERO_INIT;
    vec3 sideways_movement = GLM_VEC3_ZERO_INIT;
    vec3 vertical_movement = GLM_VEC3_ZERO_INIT;

    // Forwards camera forward_movement
    if (keys[SDL_SCANCODE_W])
    {
        glm_vec3_normalize_to(game_globals.camera.forward, forward_movement);
        glm_vec3_abs(forward_movement, forward_movement);
        glm_vec3_scale(
            game_globals.camera.forward,
            (forward_movement[0] > forward_movement[1] && forward_movement[0] > forward_movement[2]) ? 1.0f / forward_movement[0] :
            (forward_movement[1] > forward_movement[0] && forward_movement[1] > forward_movement[2]) ? 1.0f / forward_movement[1] :
            (forward_movement[2] > forward_movement[0] && forward_movement[2] > forward_movement[1]) ? 1.0f / forward_movement[2] :
            1.0f,
            forward_movement);
        
        // Apply forwards forward_movement speed
        glm_vec3_scale(forward_movement, 2.25f, forward_movement);
    }
    
    // Backwards camera forward_movement
    if (keys[SDL_SCANCODE_S])
    {
        glm_vec3_normalize_to(game_globals.camera.forward, forward_movement);
        glm_vec3_abs(forward_movement, forward_movement);
        glm_vec3_scale(
            game_globals.camera.forward,
            (forward_movement[0] > forward_movement[1] && forward_movement[0] > forward_movement[2]) ? 1.0f / forward_movement[0] :
            (forward_movement[1] > forward_movement[0] && forward_movement[1] > forward_movement[2]) ? 1.0f / forward_movement[1] :
            (forward_movement[2] > forward_movement[0] && forward_movement[2] > forward_movement[1]) ? 1.0f / forward_movement[2] :
            1.0f,
            forward_movement);
        
        // Apply backwards forward_movement speed
        glm_vec3_scale(forward_movement, -2.0f, forward_movement);
    }

    // Left camera sideways_movement
    if (keys[SDL_SCANCODE_A])
    {
        glm_vec3_normalize_to(game_globals.camera.right, sideways_movement);
        glm_vec3_abs(sideways_movement, sideways_movement);
        glm_vec3_scale(
            game_globals.camera.right,
            (sideways_movement[0] > sideways_movement[1] && sideways_movement[0] > sideways_movement[2]) ? 1.0f / sideways_movement[0] :
            (sideways_movement[1] > sideways_movement[0] && sideways_movement[1] > sideways_movement[2]) ? 1.0f / sideways_movement[1] :
            (sideways_movement[2] > sideways_movement[0] && sideways_movement[2] > sideways_movement[1]) ? 1.0f / sideways_movement[2] :
            1.0f,
            sideways_movement);
        
        // Apply sideways sideways_movement speed
        glm_vec3_scale(sideways_movement, 2.0f, sideways_movement);
    }
    
    // Right camera sideways_movement
    if (keys[SDL_SCANCODE_D])
    {
        glm_vec3_normalize_to(game_globals.camera.right, sideways_movement);
        glm_vec3_abs(sideways_movement, sideways_movement);
        glm_vec3_scale(
            game_globals.camera.right,
            (sideways_movement[0] > sideways_movement[1] && sideways_movement[0] > sideways_movement[2]) ? 1.0f / sideways_movement[0] :
            (sideways_movement[1] > sideways_movement[0] && sideways_movement[1] > sideways_movement[2]) ? 1.0f / sideways_movement[1] :
            (sideways_movement[2] > sideways_movement[0] && sideways_movement[2] > sideways_movement[1]) ? 1.0f / sideways_movement[2] :
            1.0f,
            sideways_movement);
        
        // Apply sideways sideways_movement speed
        glm_vec3_scale(sideways_movement, -2.0f, sideways_movement);
    }

    // Upward camera vertical_movement
    if (keys[SDL_SCANCODE_R])
    {
        glm_vec3_add(vertical_movement, (vec3){0, 0, 2}, vertical_movement);
    }

    // Downward camera vertical_movement
    if (keys[SDL_SCANCODE_F] && !keys[SDL_SCANCODE_R])
    {
        glm_vec3_sub(vertical_movement, (vec3){0, 0, 2}, vertical_movement);
    }

    vec3 movement;
    glm_vec3_mix(forward_movement, sideways_movement, 0.5f, movement);
    glm_vec3_scale(movement, 2.0f, movement);
    movement[2] = 0.0f;
    glm_vec3_mix(vertical_movement, movement, 0.5f, movement);
    glm_vec3_scale(movement, 2.0f, movement);

    // Double movement amount if either shift key is down
    if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT])
        glm_vec3_scale(movement, 2.0f, movement);
    
    // Scale the movement amount by the camera's movement speed per tick
    glm_vec3_scale(movement, game_globals.camera_movement_speed * delta_ticks, movement);
    
    // Add the movement amount to the camera velocity
    glm_vec3_copy(movement, game_globals.camera.velocity);

    // Update the view model animations based on the ground plane movement amount
    vec3 ground_movement;
    glm_vec3_copy(movement, ground_movement);
    glm_vec3_normalize(ground_movement);
    float movement_amount = glm_vec3_norm(ground_movement);
    
    struct object_data *weapon_object = object_get_data(game_globals.weapon_object_index);
    
    int moving_animation_index = model_find_animation_by_name(weapon_object->model_index, "first_person moving");
    bool moving_animation_active = animation_manager_is_animation_active(&weapon_object->animations, moving_animation_index);

    if (!moving_animation_active && movement_amount != 0.0f)
        animation_manager_set_animation_active(&weapon_object->animations, moving_animation_index, moving_animation_active = true);
    else if (moving_animation_active && movement_amount == 0.0f)
        animation_manager_set_animation_active(&weapon_object->animations, moving_animation_index, moving_animation_active = false);

    animation_manager_set_animation_state_speed(&weapon_object->animations, moving_animation_index, movement_amount);

    // Apply the camera updates
    camera_update(&game_globals.camera);
}

static void game_update_objects(void)
{
    // --------------------------------------------------------------------------------
    // Grunt object updates
    // --------------------------------------------------------------------------------

    struct object_data *grunt_object = object_get_data(game_globals.grunt_object_index);

    if (!animation_manager_is_animation_active(&grunt_object->animations, 0))
    {
        animation_manager_set_animation_active(&grunt_object->animations, 0, true);
    }

    // --------------------------------------------------------------------------------
    // First person weapon object updates
    // --------------------------------------------------------------------------------

    struct object_data *weapon_object = object_get_data(game_globals.weapon_object_index);

    int ready_animation_index = model_find_animation_by_name(weapon_object->model_index, "first_person ready");
    bool ready_animation_active = animation_manager_is_animation_active(&weapon_object->animations, ready_animation_index);

    int reload_empty_animation_index = model_find_animation_by_name(weapon_object->model_index, "first_person reload_empty");
    bool reload_empty_animation_active = animation_manager_is_animation_active(&weapon_object->animations, reload_empty_animation_index);

    int melee_strike_1_animation_index = model_find_animation_by_name(weapon_object->model_index, "first_person melee_strike_1");
    bool melee_strike_1_animation_active = animation_manager_is_animation_active(&weapon_object->animations, melee_strike_1_animation_index);
    
    // Move the view model to the camera position + camera velocity
    glm_vec3_copy(game_globals.camera.position, weapon_object->position);

    // Apply the view model position offset
    glm_vec3_add(weapon_object->position, (vec3){0.0f, 0.0f, -0.015f}, weapon_object->position);

    // Rotate the view model in the same direction as the camera
    glm_vec3_copy((vec3){0.0f, -game_globals.camera.rotation[1], game_globals.camera.rotation[0]}, weapon_object->rotation);

    // Play the ready animation at startup if it hasn't already played
    if (!TEST_BIT(game_globals.flags, _game_played_initial_ready_animation_bit))
    {
        SET_BIT(game_globals.flags, _game_played_initial_ready_animation_bit, true);
        animation_manager_set_animation_active(&weapon_object->animations, ready_animation_index, ready_animation_active = true);
    }

    const uint8_t *keys = SDL_GetKeyboardState(NULL);

    // Manual animation playback 1
    if (keys[SDL_SCANCODE_1])
    {
        SET_BIT(game_globals.flags, _game_input_1_bit, true);
    }
    else if (TEST_BIT(game_globals.flags, _game_input_1_bit))
    {
        SET_BIT(game_globals.flags, _game_input_1_bit, false);
        animation_manager_set_animation_active(&weapon_object->animations, ready_animation_index, ready_animation_active = true);
    }

    // Manual animation playback 2
    if (keys[SDL_SCANCODE_2])
    {
        SET_BIT(game_globals.flags, _game_input_2_bit, true);
    }
    else if (TEST_BIT(game_globals.flags, _game_input_2_bit))
    {
        SET_BIT(game_globals.flags, _game_input_2_bit, false);
        animation_manager_set_animation_active(&weapon_object->animations, reload_empty_animation_index, reload_empty_animation_active = true);
    }

    // Manual animation playback 3
    if (keys[SDL_SCANCODE_3])
    {
        SET_BIT(game_globals.flags, _game_input_3_bit, true);
    }
    else if (TEST_BIT(game_globals.flags, _game_input_3_bit))
    {
        SET_BIT(game_globals.flags, _game_input_3_bit, false);
        animation_manager_set_animation_active(&weapon_object->animations, melee_strike_1_animation_index, melee_strike_1_animation_active = true);
    }

    // --------------------------------------------------------------------------------
    // Flashlight updates
    // --------------------------------------------------------------------------------

    struct light_data *light = light_get_data(game_globals.flashlight_light_index);
    glm_vec3_copy(game_globals.camera.position, light->position);
    glm_vec3_copy(game_globals.camera.forward, light->direction);
    
    vec3 light_offset = { 0.0f, 0.0f, 0.0f };
    glm_vec3_copy(light->direction, light_offset);
    glm_vec3_mul(light_offset, (vec3){0.25f, 0.25f, 0.25f}, light_offset);

    glm_vec3_add(light->position, light_offset, light->position);

    if (keys[SDL_SCANCODE_H])
    {
        SET_BIT(game_globals.flags, _game_input_h_bit, true);
    }
    else if (TEST_BIT(game_globals.flags, _game_input_h_bit))
    {
        SET_BIT(game_globals.flags, _game_input_h_bit, false);
        bool light_is_hidden = TEST_BIT(light->flags, _light_is_hidden_bit);
        SET_BIT(light->flags, _light_is_hidden_bit, !light_is_hidden);
    }
}
