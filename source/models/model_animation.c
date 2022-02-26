/*
MODEL_ANIMATION.C
    Model animation management.
*/

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "models.h"

void model_animation_manager_initialize(
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

void model_animation_manager_dispose(
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

void model_animation_manager_set_animation_active(
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

void model_animation_manager_update(
    struct model_animation_manager *manager,
    float delta_ticks)
{
    assert(manager);

    // TODO: update active animations
}
