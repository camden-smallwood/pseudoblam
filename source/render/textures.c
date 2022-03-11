#include "render/textures.h"

/* ---------- private variables */

struct
{
    int texture_count;
    struct texture_data *textures;
} static texture_globals;

/* ---------- public code */

int texture_new(
    enum texture_type type,
    int width,
    int height)
{
    // TODO
}

void texture_delete(
    int texture_index)
{
    // TODO
}

const char *texture_type_to_string(
    enum texture_type type)
{
    switch (type)
    {
    case _texture_type_1d:
        return "1d";

    case _texture_type_2d:
        return "2d";

    case _texture_type_3d:
        return "3d";

    case _texture_type_cube:
        return "cube";

    case _texture_type_array:
        return "array";
    }
}

struct texture_data *texture_get_data(
    int texture_index)
{
    // TODO
}

void texture_resize(
    int texture_index,
    int width,
    int height)
{
    // TODO
}
