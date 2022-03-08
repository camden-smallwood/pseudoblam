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
