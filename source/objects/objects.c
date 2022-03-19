/*
OBJECTS.C
    Object management code.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "objects/objects.h"

/* ---------- private variables */

struct
{
    int object_count;
    struct object_data *objects;
} static object_globals;

/* ---------- public code */

void objects_initialize(void)
{
    memset(&object_globals, 0, sizeof(object_globals));
}

void objects_dispose(void)
{
    for (int object_index = 0; object_index < object_globals.object_count; object_index++)
    {
        object_delete(object_index);
    }

    free(object_globals.objects);
}

void objects_update(float delta_ticks)
{
    struct object_iterator iterator;
    object_iterator_new(&iterator);

    while (object_iterator_next(&iterator) != -1)
    {
        struct object_data *object = iterator.data;

        model_animations_update(&object->animations, delta_ticks);
        // TODO: compute node transforms from position/rotation/scale/animations
    }
}

int object_new(void)
{
    struct object_data object;
    memset(&object, 0, sizeof(object));

    glm_vec3_copy((vec3){1, 1, 1}, object.scale);
    
    int object_index = object_globals.object_count;
    mempush(&object_globals.object_count, (void **)&object_globals.objects, &object, sizeof(object), realloc);
    
    return object_index;
}

void object_delete(int object_index)
{
    struct object_data *object = object_get_data(object_index);
    assert(object);

    // TODO
}

struct object_data *object_get_data(int object_index)
{
    if (object_index == -1)
        return NULL;
    
    assert(object_index >= 0 && object_index < object_globals.object_count);
    return object_globals.objects + object_index;
}

void object_iterator_new(struct object_iterator *iterator)
{
    assert(iterator);

    iterator->data = NULL;
    iterator->index = -1;
}

int object_iterator_next(struct object_iterator *iterator)
{
    assert(iterator);

    if (++iterator->index >= object_globals.object_count)
    {
        iterator->data = NULL;
        iterator->index = -1;
        return -1;
    }

    int object_index = iterator->index;
    iterator->data = object_globals.objects + object_index;
    
    return object_index;
}
