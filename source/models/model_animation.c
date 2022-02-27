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
}

void model_animations_update(
    struct model_animation_manager *manager,
    float delta_ticks)
{
    assert(manager);

    struct model_data *model = model_get_data(manager->model_index);
    assert(model);

    int root_node_index = model_find_root_node(model);

    for (int animation_index = 0; animation_index < model->animation_count; animation_index++)
    {
        if (BIT_VECTOR_TEST_BIT(manager->active_animations_bit_vector, animation_index))
        {
            struct model_animation *animation = model->animations + animation_index;
            struct model_animation_state *state = manager->animation_states + animation_index;

            state->time = fmodf(state->time + (animation->ticks_per_second * delta_ticks), animation->duration);
            
            model_animation_compute_node_matrices(manager, animation_index, root_node_index, GLM_MAT4_IDENTITY);
        }
    }
}

/* ---------- private code */

static void model_animation_compute_node_matrices(
    struct model_animation_manager *manager,
    int animation_index,
    int node_index,
    mat4 parent_transform)
{
    struct model_data *model = model_get_data(manager->model_index);
    assert(animation_index >= 0 && animation_index < model->animation_count);

    struct model_animation *animation = model->animations + animation_index;
    struct model_animation_state *state = manager->animation_states + animation_index;

    struct model_node *node = model->nodes + node_index;
    
    mat4 node_transform;
    glm_mat4_copy(node->transform, node_transform);

    for (int channel_index = 0; channel_index < animation->channel_count; channel_index++)
    {
        struct model_animation_channel *channel = animation->channels + channel_index;
        
        switch (channel->type)
        {
        case _model_animation_channel_type_node:
            if (channel->node_index == node_index)
            {
                // TODO: apply positions to node_transform
                // TODO: apply rotations to node_transform
                // TODO: apply scalings to node_transform

                fprintf(stderr, "ERROR: node channel animations not implemented\n");
                exit(EXIT_FAILURE);
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

    mat4 global_transform;
    glm_mat4_mul(parent_transform, node_transform, global_transform);

    mat4 *node_matrix = state->node_matrices + node_index;
    // TODO: apply bone offset to global_transform

    for (int child_node_index = node->first_child_index;
        child_node_index != -1;
        child_node_index = model->nodes[child_node_index].next_sibling_index)
    {
        model_animation_compute_node_matrices(manager, animation_index, child_node_index, global_transform);
        // TODO: compute child node matrices
    }
}
