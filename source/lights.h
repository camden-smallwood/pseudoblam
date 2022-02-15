#pragma once
#include <cglm/cglm.h>

/* ---------- constants */

enum light_type
{
    _light_type_directional,
    _light_type_point,
    _light_type_spot,
};

/* ---------- structures */

struct light_data
{
    enum light_type type;

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

/* ---------- prototypes/LIGHTS.C */

void lights_initialize(void);
void lights_dispose(void);
