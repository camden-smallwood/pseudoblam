/*
OBJECTS.H
    Object management declarations.
*/

#pragma once

#include <cglm/cglm.h>

#include "models/models.h"

/* ---------- constants */

enum object_flags
{
    NUMBER_OF_OBJECT_FLAGS
};

/* ---------- structures */

struct object_data
{
    char name[32];

    unsigned int flags;

    vec3 position;
    vec3 rotation;
    vec3 scale;

    int model_index;
    struct model_animation_manager animations;
    mat4 node_matrices[MAXIMUM_NUMBER_OF_MODEL_NODES];
};

struct object_iterator
{
    struct object_data *data;
    int index;
};

/* ---------- prototypes/OBJECTS.C */

void objects_initialize(void);
void objects_dispose(void);
void objects_update(float delta_ticks);

int object_new(void);
void object_delete(int object_index);

struct object_data *object_get_data(int object_index);

void object_iterator_new(struct object_iterator *iterator);
int object_iterator_next(struct object_iterator *iterator);
