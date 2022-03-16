/*
TEXTURES.H
    Texture management and rendering declarations.
*/

#pragma once
#include <stdbool.h>

/* ---------- constants */

enum texture_type
{
    _texture_type_2d,
    _texture_type_3d,
    NUMBER_OF_TEXTURE_TYPES
};

/* ---------- structures */

struct texture_data
{
    enum texture_type type;
    
    int internal_format;
    int pixel_format;
    int pixel_type;

    int samples;
    int width;
    int height;
    int depth;

    unsigned int id;
};

/* ---------- prototypes/TEXTURES.C */

void textures_initialize(void);
void textures_dispose(void);

const char *texture_type_to_string(enum texture_type type);

int texture_new(enum texture_type type, int internal_format, int pixel_format, int pixel_type, int samples, int width, int height, int depth);
void texture_delete(int texture_index);

struct texture_data *texture_get_data(int texture_index);

int texture_get_target(int texture_index);

void texture_resize(int texture_index, int samples, int width, int height, int depth);
void texture_set_image_data(int texture_index, void *data);
