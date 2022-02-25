/*
ANIMATIONS.C
    Animation management code.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <assimp/cimport.h>
#include <assimp/anim.h>
#include <assimp/scene.h>

#include "common.h"
#include "animations.h"

/* ---------- private variables */

struct
{
    int animation_graph_count;
    struct animation_graph_data *animation_graphs;
} static animation_globals;

/* ---------- public code */

void animations_initialize(void)
{
    memset(&animation_globals, 0, sizeof(animation_globals));
}

void animations_dispose(void)
{
    for (int animation_graph_index = 0;
        animation_graph_index < animation_globals.animation_graph_count;
        animation_graph_index++)
    {
        animation_graph_delete(animation_graph_index);
    }
}

void animations_update(float delta_ticks)
{
    // TODO
}

int animation_graph_new(void)
{
    int animation_graph_index = animation_globals.animation_graph_count;

    struct animation_graph_data animation_graph =
    {
        .animation_count = 0,
        .animations = NULL,
    };

    mempush(
        &animation_globals.animation_graph_count,
        (void **)&animation_globals.animation_graphs,
        &animation_graph,
        sizeof(animation_graph),
        realloc);
    
    return animation_graph_index;
}

void animation_graph_delete(int animation_graph_index)
{
    assert(animation_graph_index >= 0 && animation_graph_index < animation_globals.animation_graph_count);
    struct animation_graph_data *animation_graph = animation_globals.animation_graphs + animation_graph_index;

    for (int animation_index = 0; animation_index < animation_graph->animation_count; animation_index++)
    {
        struct animation_data *animation = animation_graph->animations + animation_index;
        assert(animation);

        // TODO
    }

    free(animation_graph->animations);
}

struct animation_graph_data *animation_graph_get_data(int animation_graph_index)
{
    if (animation_graph_index == -1)
        return NULL;
    
    assert(animation_graph_index >= 0 && animation_graph_index < animation_globals.animation_graph_count);
    return animation_globals.animation_graphs + animation_graph_index;
}
