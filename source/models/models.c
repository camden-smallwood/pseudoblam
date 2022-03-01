/*
MODELS.C
    3D model management.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "models/models.h"

/* ---------- private variables */

struct
{
    int model_count;
    struct model_data *models;
} static model_globals;

/* ---------- public code */

void models_initialize(void)
{
    memset(&model_globals, 0, sizeof(model_globals));
}

void models_dispose(void)
{
    for (int model_index = 0; model_index < model_globals.model_count; model_index++)
    {
        model_delete(model_index);
    }

    free(model_globals.models);
}

int model_new(void)
{
    struct model_data model;
    memset(&model, 0, sizeof(model));
    
    int model_index = model_globals.model_count;
    mempush(&model_globals.model_count, (void **)&model_globals.models, &model, sizeof(model), realloc);
    
    return model_index;
}

void model_delete(int model_index)
{
    struct model_data *model = model_get_data(model_index);
    assert(model);

    // TODO
}

struct model_data *model_get_data(int model_index)
{
    assert(model_index >= 0 && model_index < model_globals.model_count);
    return model_globals.models + model_index;
}

void model_iterator_new(struct model_iterator *iterator)
{
    assert(iterator);

    iterator->data = NULL;
    iterator->index = -1;
}

int model_iterator_next(struct model_iterator *iterator)
{
    assert(iterator);

    if (++iterator->index >= model_globals.model_count)
    {
        iterator->data = NULL;
        iterator->index = -1;
        return -1;
    }

    int model_index = iterator->index;
    iterator->data = model_globals.models + model_index;
    
    return model_index;
}

int model_find_root_node(
    struct model_data *model)
{
    assert(model);

    for (int node_index = 0; node_index < model->node_count; node_index++)
    {
        struct model_node *node = model->nodes + node_index;

        if (node->parent_index == -1)
            return node_index;
    }

    return -1;
}

int model_find_node_by_name(
    struct model_data *model,
    const char *node_name)
{
    assert(model);
    assert(node_name);

    for (int node_index = 0; node_index < model->node_count; node_index++)
    {
        struct model_node *node = model->nodes + node_index;

        if (strcmp(node_name, node->name) == 0)
            return node_index;
    }

    return -1;
}

int model_node_add_child_node(
    struct model_data *model,
    int node_index,
    struct model_node *child_node)
{
    int child_node_index = model->node_count;
    mempush(&model->node_count, (void **)&model->nodes, child_node, sizeof(*child_node), realloc);

    if (node_index == -1)
        return child_node_index;

    struct model_node *node = model->nodes + node_index;

    if (node->first_child_index == -1)
    {
        node->first_child_index = child_node_index;
        return child_node_index;
    }

    for (node = model->nodes + node->first_child_index; ; node = model->nodes + node->next_sibling_index)
    {
        if (node->next_sibling_index == -1)
        {
            node->next_sibling_index = child_node_index;
            break;
        }
    }
    
    return child_node_index;
}
