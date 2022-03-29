/*
ANIMATION_MANAGER.C
    Animation management code.
*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "models/models.h"
#include "animations/animation_data.h"
#include "animations/animation_manager.h"

/* ---------- private prototypes */

static void animation_manager_update_animation(
    struct animation_manager *manager,
    struct model_data *model,
    int animation_index,
    float delta_ticks);

static void animation_manager_compute_node_orientations(
    struct model_data *model,
    struct animation_data *animation,
    struct animation_state *state);

static void animation_manager_compute_node_matrices(
    struct animation_manager *manager,
    struct model_data *model,
    int node_index,
    mat4 transform);

/* ---------- public code */

void animation_manager_initialize(
    struct animation_manager *manager,
    int model_index)
{
    assert(manager);
    memset(manager, 0, sizeof(*manager));

    struct model_data *model = model_get_data(model_index);
    
    if (!model)
    {
        return;
    }

    manager->model_index = model_index;
    assert(manager->active_animations_bit_vector = calloc(BIT_VECTOR_LENGTH_IN_WORDS(model->animation_count), sizeof(unsigned int)));
    assert(manager->states = calloc(model->animation_count, sizeof(*manager->states)));
    assert(manager->blended_node_matrices = calloc(model->node_count, sizeof(*manager->blended_node_matrices)));
    
    for (int state_index = 0; state_index < model->animation_count; state_index++)
    {
        struct animation_state *state = manager->states + state_index;
        
        state->time = 0.0f;
        state->speed = 1.0f;

        assert(state->node_states = calloc(model->node_count, sizeof(*state->node_states)));
    }
}

void animation_manager_dispose(
    struct animation_manager *manager)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    for (int state_index = 0; state_index < model->animation_count; state_index++)
    {
        struct animation_state *state = manager->states + state_index;
        free(state->node_states);
    }

    free(manager->active_animations_bit_vector);
    free(manager->states);
    free(manager->blended_node_matrices);
}

void animation_manager_set_animation_flags(
    struct animation_manager *manager,
    int animation_index,
    unsigned int flags)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);
    
    manager->states[animation_index].flags = flags;
}

bool animation_manager_is_animation_looping(
    struct animation_manager *manager,
    int animation_index)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);

    return TEST_BIT(manager->states[animation_index].flags, _animation_state_looping_bit);
}

void animation_manager_set_animation_looping(
    struct animation_manager *manager,
    int animation_index,
    bool looping)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);

    SET_BIT(manager->states[animation_index].flags, _animation_state_looping_bit, looping);
}

bool animation_manager_is_animation_paused(
    struct animation_manager *manager,
    int animation_index)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);

    return TEST_BIT(manager->states[animation_index].flags, _animation_state_paused_bit);
}

void animation_manager_set_animation_paused(
    struct animation_manager *manager,
    int animation_index,
    bool paused)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);

    SET_BIT(manager->states[animation_index].flags, _animation_state_paused_bit, paused);
}

bool animation_manager_is_animation_active(
    struct animation_manager *manager,
    int animation_index)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    
    if (!model)
    {
        return false;
    }

    assert(animation_index >= 0 && animation_index < model->animation_count);

    return BIT_VECTOR_TEST_BIT(manager->active_animations_bit_vector, animation_index) != 0;
}

void animation_manager_set_animation_active(
    struct animation_manager *manager,
    int animation_index,
    bool active)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);
    
    BIT_VECTOR_SET_BIT(manager->active_animations_bit_vector, animation_index, active);
    manager->active_animation_count += active ? 1 : -1;

    manager->states[animation_index].time = 0.0f;
}

void animation_manager_set_animation_state_time(
    struct animation_manager *manager,
    int animation_index,
    float time)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    manager->states[animation_index].time = time;
}

void animation_manager_set_animation_state_speed(
    struct animation_manager *manager,
    int animation_index,
    float speed)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    manager->states[animation_index].speed = speed;
}

void animation_manager_update(
    struct animation_manager *manager,
    float delta_ticks)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);

    if (!model)
        return;
    
    for (int animation_index = 0; animation_index < model->animation_count; animation_index++)
    {
        animation_manager_update_animation(manager, model, animation_index, delta_ticks);
    }

    if (!model->nodes)
        return;
    
    int root_node_index = model_get_root_node(model);
    animation_manager_compute_node_matrices(manager, model, root_node_index, GLM_MAT4_IDENTITY);
}

/* ---------- private code */

static void animation_manager_update_animation(
    struct animation_manager *manager,
    struct model_data *model,
    int animation_index,
    float delta_ticks)
{
    struct animation_data *animation = model->animations + animation_index;
    struct animation_state *state = manager->states + animation_index;
    
    if (BIT_VECTOR_TEST_BIT(manager->active_animations_bit_vector, animation_index))
    {
        state->time += (animation->ticks_per_second * state->speed) * delta_ticks;

        if (TEST_BIT(state->flags, _animation_state_looping_bit))
        {
            state->time = fmodf(state->time, animation->duration);
        }
        else if (state->time >= animation->duration)
        {
            state->time = 0.0f;
            BIT_VECTOR_SET_BIT(manager->active_animations_bit_vector, animation_index, 0);
            assert(manager->active_animation_count--);
        }
    }

    animation_manager_compute_node_orientations(model, animation, state);
}

static void animation_manager_compute_node_orientations(
    struct model_data *model,
    struct animation_data *animation,
    struct animation_state *state)
{
    for (int node_index = 0; node_index < model->node_count; node_index++)
    {
        struct model_node *node = model->nodes + node_index;
        struct animation_node_state *node_state = state->node_states + node_index;

        int animation_count = 0;

        mat4 position_matrix = GLM_MAT4_IDENTITY_INIT;
        mat4 rotation_matrix = GLM_MAT4_IDENTITY_INIT;
        mat4 scaling_matrix = GLM_MAT4_IDENTITY_INIT;

        for (int channel_index = 0; channel_index < animation->channel_count; channel_index++)
        {
            struct animation_channel *channel = animation->channels + channel_index;
            assert(channel->type == _animation_channel_type_node);

            if (channel->node_index != node_index)
                continue;

            if (channel->position_key_count == 1)
            {
                mat4 current_position_matrix;
                glm_mat4_identity(current_position_matrix);
                glm_translate(current_position_matrix, channel->position_keys[0].position);
                glm_mat4_mul(position_matrix, current_position_matrix, position_matrix);
                animation_count++;
            }
            else
            {
                for (int position_key_index = 0; position_key_index < channel->position_key_count; position_key_index++)
                {
                    struct animation_position_key *position_key = channel->position_keys + position_key_index;

                    int next_position_key_index = position_key_index + 1;
                    if (next_position_key_index >= channel->position_key_count)
                        next_position_key_index = 0;
                    
                    struct animation_position_key *next_position_key = channel->position_keys + next_position_key_index;

                    if (state->time < position_key->time)
                    {
                        float scale_factor = (state->time - position_key->time) / (next_position_key->time - position_key->time);

                        vec3 interpolated_position;
                        glm_vec3_mix(position_key->position, next_position_key->position, scale_factor, interpolated_position);

                        mat4 current_position_matrix;
                        glm_mat4_identity(current_position_matrix);
                        glm_translate(current_position_matrix, interpolated_position);
                        glm_mat4_mul(position_matrix, current_position_matrix, position_matrix);
                        animation_count++;
                        break;
                    }
                }
            }

            if (channel->rotation_key_count == 1)
            {
                mat4 current_rotation_matrix;
                glm_mat4_identity(current_rotation_matrix);
                glm_quat_mat4(channel->rotation_keys[0].rotation, current_rotation_matrix);
                glm_mat4_mul(rotation_matrix, current_rotation_matrix, rotation_matrix);
                animation_count++;
            }
            else
            {
                for (int rotation_key_index = 0; rotation_key_index < channel->rotation_key_count; rotation_key_index++)
                {
                    struct animation_rotation_key *rotation_key = channel->rotation_keys + rotation_key_index;

                    int next_rotation_key_index = rotation_key_index + 1;
                    if (next_rotation_key_index >= channel->rotation_key_count)
                        next_rotation_key_index = 0;
                    
                    struct animation_rotation_key *next_rotation_key = channel->rotation_keys + next_rotation_key_index;

                    if (state->time < rotation_key->time)
                    {
                        float scale_factor = (state->time - rotation_key->time) / (next_rotation_key->time - rotation_key->time);

                        vec4 interpolated_rotation;
                        glm_quat_slerp(rotation_key->rotation, next_rotation_key->rotation, scale_factor, interpolated_rotation);

                        mat4 current_rotation_matrix;
                        glm_mat4_identity(current_rotation_matrix);
                        glm_quat_mat4(interpolated_rotation, current_rotation_matrix);
                        glm_mat4_mul(rotation_matrix, current_rotation_matrix, rotation_matrix);
                        animation_count++;
                        break;
                    }
                }
            }

            if (channel->scaling_key_count == 1)
            {
                mat4 current_scaling_matrix;
                glm_mat4_identity(current_scaling_matrix);
                glm_scale(current_scaling_matrix, channel->scaling_keys[0].scaling);
                glm_mat4_mul(scaling_matrix, current_scaling_matrix, scaling_matrix);
                animation_count++;
            }
            else
            {
                for (int scaling_key_index = 0; scaling_key_index < channel->scaling_key_count; scaling_key_index++)
                {
                    struct animation_scaling_key *scaling_key = channel->scaling_keys + scaling_key_index;

                    int next_scaling_key_index = scaling_key_index + 1;
                    if (next_scaling_key_index >= channel->scaling_key_count)
                        next_scaling_key_index = 0;
                    
                    struct animation_scaling_key *next_scaling_key = channel->scaling_keys + next_scaling_key_index;

                    if (state->time < scaling_key->time)
                    {
                        float scale_factor = (state->time - scaling_key->time) / (next_scaling_key->time - scaling_key->time);

                        vec3 interpolated_scaling;
                        glm_vec3_mix(scaling_key->scaling, next_scaling_key->scaling, scale_factor, interpolated_scaling);

                        mat4 current_scaling_matrix;
                        glm_mat4_identity(current_scaling_matrix);
                        glm_scale(current_scaling_matrix, interpolated_scaling);
                        glm_mat4_mul(scaling_matrix, current_scaling_matrix, scaling_matrix);
                        animation_count++;
                        break;
                    }
                }
            }
        }

        mat4 local_transform;

        if (animation_count == 0)
        {
            glm_mat4_copy(node->default_transform, local_transform);
        }
        else
        {
            glm_mul(scaling_matrix, rotation_matrix, local_transform);
            glm_mul(position_matrix, local_transform, local_transform);
        }

        glm_decompose(local_transform, node_state->position, rotation_matrix, node_state->scale);
        glm_mat4_quat(rotation_matrix, node_state->rotation);
    }
}

static void animation_manager_compute_node_matrices(
    struct animation_manager *manager,
    struct model_data *model,
    int node_index,
    mat4 parent_transform)
{
    struct model_node *node = model->nodes + node_index;

    int animation_count = 0;

    vec3 total_position;
    vec4 total_rotation;
    vec3 total_scale;

    for (int animation_index = 0; animation_index < model->animation_count; animation_index++)
    {
        if (!animation_manager_is_animation_active(manager, animation_index))
            continue;
        
        struct animation_state *state = manager->states + animation_index;
        struct animation_node_state *node_state = state->node_states + node_index;

        if (animation_count == 0)
        {
            glm_vec3_copy(node_state->position, total_position);
            glm_vec4_copy(node_state->rotation, total_rotation);
            glm_vec3_copy(node_state->scale, total_scale);
        }
        else
        {
            glm_vec3_mix(node_state->position, total_position, 0.5f, total_position);
            glm_quat_slerp(node_state->rotation, total_rotation, 0.5f, total_rotation);
            glm_vec3_mix(node_state->scale, total_scale, 0.5f, total_scale);
        }

        animation_count++;
    }

    mat4 local_transform;

    if (animation_count == 0)
    {
        glm_mat4_copy(node->default_transform, local_transform);
    }
    else
    {
        mat4 position_matrix = GLM_MAT4_IDENTITY_INIT;
        mat4 rotation_matrix = GLM_MAT4_IDENTITY_INIT;
        mat4 scaling_matrix = GLM_MAT4_IDENTITY_INIT;

        glm_translate(position_matrix, total_position);
        glm_quat_mat4(total_rotation, rotation_matrix);
        glm_scale(scaling_matrix, total_scale);

        glm_mat4_mul(position_matrix, rotation_matrix, local_transform);
        glm_mat4_mul(local_transform, scaling_matrix, local_transform);
    }

    mat4 global_transform;
    glm_mat4_mul(parent_transform, local_transform, global_transform);

    glm_mat4_mul(global_transform, node->offset_matrix, manager->blended_node_matrices[node_index]);
    
    for (int child_node_index = node->first_child_index;
        child_node_index != -1;
        child_node_index = model->nodes[child_node_index].next_sibling_index)
    {
        animation_manager_compute_node_matrices(manager, model, child_node_index, global_transform);
    }
}
