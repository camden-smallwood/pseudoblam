/*
LIGHTS.C
    Light management code.
*/

/* ---------- headers */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "lights.h"

/* ---------- private variables */

struct
{
    int light_count;
    struct light_data *lights;
} static light_globals;

/* ---------- public code */

void lights_initialize(void)
{
    memset(&light_globals, 0, sizeof(light_globals));
}

void lights_dispose(void)
{
    free(light_globals.lights);
}

int light_new(void)
{
    struct light_data light;
    memset(&light, 0, sizeof(light));

    int light_index = light_globals.light_count;
    mempush(&light_globals.light_count, (void **)&light_globals.lights, &light, sizeof(light), realloc);

    return light_index;
}

void light_delete(int light_index)
{
    assert(light_index >= 0 && light_index < light_globals.light_count);
    // TODO
}

struct light_data *light_get_data(int light_index)
{
    if (light_index == -1)
        return NULL;
    
    assert(light_index >= 0 && light_index < light_globals.light_count);
    return light_globals.lights + light_index;
}

void light_iterator_new(struct light_iterator *iterator)
{
    assert(iterator);

    iterator->data = NULL;
    iterator->index = -1;
}

int light_iterator_next(struct light_iterator *iterator)
{
    assert(iterator);

    if (++iterator->index >= light_globals.light_count)
    {
        iterator->data = NULL;
        iterator->index = -1;
        return -1;
    }

    int light_index = iterator->index;
    iterator->data = light_globals.lights + light_index;
    
    return light_index;
}

bool light_is_hidden(int light_index)
{
    struct light_data *data = light_get_data(light_index);
    assert(data);

    return TEST_BIT(data->flags, _light_is_hidden_bit);
}

void light_set_hidden(int light_index, bool hidden)
{
    struct light_data *data = light_get_data(light_index);
    assert(data);

    SET_BIT(data->flags, _light_is_hidden_bit, hidden);
}
