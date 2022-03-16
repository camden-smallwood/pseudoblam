/*
MODEL_ANIMATION.C
    Model animation management.
*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "models/models.h"

/* ---------- private prototypes */

static void model_animation_update(
    struct model_animation_manager *manager,
    int animation_index,
    float delta_ticks);

static void model_animation_compute_node_matrices(
    struct model_animation_manager *manager,
    int animation_index,
    int node_index,
    mat4 transform);

/* ---------- public code */

void model_animations_initialize(
    struct model_animation_manager *manager,
    int model_index)
{
    assert(manager);
    memset(manager, 0, sizeof(*manager));

    struct model_data *model = model_get_data(model_index);
    assert(model);

    manager->model_index = model_index;
    assert(manager->active_animations_bit_vector = calloc(BIT_VECTOR_LENGTH_IN_WORDS(model->animation_count), sizeof(unsigned int)));
    assert(manager->animation_states = calloc(model->animation_count, sizeof(*manager->animation_states)));
    
    for (int state_index = 0; state_index < model->animation_count; state_index++)
    {
        struct model_animation_state *state = manager->animation_states + state_index;
        
        state->time = 0.0f;
        state->speed = 1.0f;

        assert(state->node_matrices = calloc(model->node_count, sizeof(*state->node_matrices)));
    }
}

void model_animations_dispose(
    struct model_animation_manager *manager)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    for (int state_index = 0; state_index < model->animation_count; state_index++)
    {
        struct model_animation_state *state = manager->animation_states + state_index;
        free(state->node_matrices);
    }

    free(manager->animation_states);
}

void model_set_animation_active(
    struct model_animation_manager *manager,
    int animation_index,
    bool active)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    assert(animation_index >= 0 && animation_index < model->animation_count);
    BIT_VECTOR_SET_BIT(manager->active_animations_bit_vector, animation_index, active);

    manager->animation_states[animation_index].time = 0.0f;
}

void model_animations_update(
    struct model_animation_manager *manager,
    float delta_ticks)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);

    for (int animation_index = 0; animation_index < model->animation_count; animation_index++)
        model_animation_update(manager, animation_index, delta_ticks);
}

/* ---------- private code */

static void model_animation_update(
    struct model_animation_manager *manager,
    int animation_index,
    float delta_ticks)
{
    if (!BIT_VECTOR_TEST_BIT(manager->active_animations_bit_vector, animation_index))
        return;
    
    struct model_data *model = model_get_data(manager->model_index);
    int root_node_index = model_find_root_node(model);

    struct model_animation *animation = model->animations + animation_index;
    struct model_animation_state *state = manager->animation_states + animation_index;

    state->time = fmodf(state->time + (animation->ticks_per_second * delta_ticks), animation->duration);
    
    model_animation_compute_node_matrices(manager, animation_index, root_node_index, GLM_MAT4_IDENTITY);
}

static void model_animation_compute_node_matrices(
    struct model_animation_manager *manager,
    int animation_index,
    int node_index,
    mat4 parent_transform)
{
    struct model_data *model = model_get_data(manager->model_index);
    struct model_node *node = model->nodes + node_index;
    struct model_animation *animation = model->animations + animation_index;
    struct model_animation_state *state = manager->animation_states + animation_index;
    
    mat4 node_transform;
    glm_mat4_identity(node_transform);
    
    int key_count = 0;

    for (int channel_index = 0; channel_index < animation->channel_count; channel_index++)
    {
        struct model_animation_channel *channel = animation->channels + channel_index;
        
        switch (channel->type)
        {
        case _model_animation_channel_type_node:
            if (channel->node_index == node_index)
            {
                if (channel->position_key_count == 1)
                {
                    mat4 position_matrix;
                    glm_mat4_identity(position_matrix);
                    glm_translate(position_matrix, channel->position_keys[0].position);
                    glm_mat4_mul(node_transform, position_matrix, node_transform);
                    key_count++;
                }
                else
                {
                    for (int position_key_index = 0; position_key_index < channel->position_key_count - 1; position_key_index++)
                    {
                        struct model_animation_position_key *position_key = channel->position_keys + position_key_index;
                        struct model_animation_position_key *next_position_key = channel->position_keys + position_key_index + 1;

                        if (state->time < position_key->time)
                        {
                            float scale_factor = (state->time - position_key->time) / (next_position_key->time - position_key->time);

                            vec3 interpolated_position;
                            glm_vec3_mix(position_key->position, next_position_key->position, scale_factor, interpolated_position);

                            mat4 position_matrix;
                            glm_mat4_identity(position_matrix);
                            glm_translate(position_matrix, interpolated_position);
                            glm_mat4_mul(node_transform, position_matrix, node_transform);
                            key_count++;
                            break;
                        }
                    }
                }

                if (channel->rotation_key_count == 1)
                {
                    mat4 rotation_matrix;
                    glm_mat4_identity(rotation_matrix);
                    glm_quat_mat4(channel->rotation_keys[0].rotation, rotation_matrix);
                    glm_mat4_mul(node_transform, rotation_matrix, node_transform);
                    key_count++;
                }
                else
                {
                    for (int rotation_key_index = 0; rotation_key_index < channel->rotation_key_count - 1; rotation_key_index++)
                    {
                        struct model_animation_rotation_key *rotation_key = channel->rotation_keys + rotation_key_index;
                        struct model_animation_rotation_key *next_rotation_key = channel->rotation_keys + rotation_key_index + 1;

                        if (state->time < rotation_key->time)
                        {
                            float scale_factor = (state->time - rotation_key->time) / (next_rotation_key->time - rotation_key->time);

                            vec4 interpolated_rotation;
                            glm_quat_slerp(rotation_key->rotation, next_rotation_key->rotation, scale_factor, interpolated_rotation);
                            glm_quat_normalize(interpolated_rotation);

                            mat4 rotation_matrix;
                            glm_mat4_identity(rotation_matrix);
                            glm_quat_mat4(interpolated_rotation, rotation_matrix);
                            glm_mat4_mul(node_transform, rotation_matrix, node_transform);
                            key_count++;
                            break;
                        }
                    }
                }

                if (channel->scaling_key_count == 1)
                {
                    mat4 scaling_matrix;
                    glm_mat4_identity(scaling_matrix);
                    glm_scale(scaling_matrix, channel->scaling_keys[0].scaling);
                    glm_mat4_mul(node_transform, scaling_matrix, node_transform);
                    key_count++;
                }
                else
                {
                    for (int scaling_key_index = 0; scaling_key_index < channel->scaling_key_count - 1; scaling_key_index++)
                    {
                        struct model_animation_scaling_key *scaling_key = channel->scaling_keys + scaling_key_index;
                        struct model_animation_scaling_key *next_scaling_key = channel->scaling_keys + scaling_key_index + 1;

                        if (state->time < scaling_key->time)
                        {
                            float scale_factor = (state->time - scaling_key->time) / (next_scaling_key->time - scaling_key->time);

                            vec3 interpolated_scaling;
                            glm_vec3_mix(scaling_key->scaling, next_scaling_key->scaling, scale_factor, interpolated_scaling);

                            mat4 scaling_matrix;
                            glm_mat4_identity(scaling_matrix);
                            glm_scale(scaling_matrix, interpolated_scaling);
                            glm_mat4_mul(node_transform, scaling_matrix, node_transform);
                            key_count++;
                            break;
                        }
                    }
                }
            }
            break;
        
        case _model_animation_channel_type_mesh:
            // TODO: implement
            fprintf(stderr, "ERROR: mesh channel animations not implemented\n");
            exit(EXIT_FAILURE);
        
        case _model_animation_channel_type_morph:
            // TODO: implement
            fprintf(stderr, "ERROR: morph channel animations not implemented\n");
            exit(EXIT_FAILURE);
        
        default:
            fprintf(stderr, "ERROR: unhandled animation channel type %i\n", channel->type);
            exit(EXIT_FAILURE);
        }
    }

    if (!key_count)
        glm_mat4_copy(node->transform, node_transform);
    
    mat4 global_transform;
    glm_mat4_mul(parent_transform, node_transform, global_transform);

    glm_mat4_mul(global_transform, node->offset_matrix, state->node_matrices[node_index]);

    // mat4 m;
    // glm_mat4_copy(state->node_matrices[node_index], m);

    // printf("node matrix of \"%s\":\n", node->name);
    // printf("00: %f, 01: %f, 02: %f, 03: %f,\n", m[0][0], m[0][1], m[0][2], m[0][3]);
    // printf("10: %f, 11: %f, 12: %f, 13: %f,\n", m[1][0], m[1][1], m[1][2], m[1][3]);
    // printf("20: %f, 21: %f, 22: %f, 23: %f,\n", m[2][0], m[2][1], m[2][2], m[2][3]);
    // printf("30: %f, 31: %f, 32: %f, 33: %f,\n", m[3][0], m[3][1], m[3][2], m[3][3]);
    // printf("\n");
    
    for (int child_node_index = node->first_child_index;
        child_node_index != -1;
        child_node_index = model->nodes[child_node_index].next_sibling_index)
    {
        model_animation_compute_node_matrices(manager, animation_index, child_node_index, global_transform);
    }
}
