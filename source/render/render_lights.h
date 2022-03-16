/*
RENDER_LIGHTS.H
    Light management and rendering declarations.
*/

#pragma once
#include <stdbool.h>
#include <cglm/cglm.h>

/* ---------- constants */

enum light_type
{
    _light_type_directional,
    _light_type_point,
    _light_type_spot,
    NUMBER_OF_LIGHT_TYPES
};

enum light_flags
{
    _light_is_hidden_bit,
    NUMBER_OF_LIGHT_FLAGS
};

/* ---------- structures */

struct light_data
{
    enum light_type type;
    unsigned int flags;

    vec3 position;
    vec3 direction;

    vec3 diffuse_color;
    vec3 ambient_color;
    vec3 specular_color;

    float constant;
    float linear;
    float quadratic;
    
    float inner_cutoff;
    float outer_cutoff;
};

struct light_iterator
{
    struct light_data *data;
    int index;
};

/* ---------- prototypes/RENDER_LIGHTS.C */

void lights_initialize(void);
void lights_dispose(void);

int light_new(void);
void light_delete(int light_index);

struct light_data *light_get_data(int light_index);

void light_iterator_new(struct light_iterator *iterator);
int light_iterator_next(struct light_iterator *iterator);

bool light_is_hidden(int light_index);
void light_set_hidden(int light_index, bool hidden);
