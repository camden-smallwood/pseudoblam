#pragma once
#include <stdbool.h>

/* ---------- constants */

enum texture_type
{
    _texture_type_1d,
    _texture_type_2d,
    _texture_type_3d,
    _texture_type_cube,
    _texture_type_array,
    NUMBER_OF_TEXTURE_TYPES
};

enum texture_flags
{
    _texture_has_multiple_samples_bit,
    NUMBER_OF_TEXTURE_FLAGS
};

/* ---------- structures */

struct texture_data
{
    enum texture_type type;
    unsigned int flags;

    int width;
    int height;

    unsigned int id;
};

/* ---------- prototypes/TEXTURES.C */

int texture_new(enum texture_type type, int width, int height);
void texture_delete(int texture_index);

const char *texture_type_to_string(enum texture_type type);

struct texture_data *texture_get_data(int texture_index);

void texture_resize(int texture_index, int width, int height);
