#pragma once
#include <GL/glew.h>

/* ---------- constants */

enum texture_type
{
    _texture_type_2d,
    _texture_type_3d,
    _texture_type_cube,
    _texture_type_array,
    NUMBER_OF_TEXTURE_TYPES
};

enum texture_image_type
{
    _texture_image_type_color,
    _texture_image_type_depth,
    _texture_image_type_depth_stencil,
    _texture_image_type_stencil,
    NUMBER_OF_TEXTURE_IMAGE_TYPES
};

enum texture_image_format
{
    // TODO
    NUMBER_OF_TEXTURE_IMAGE_FORMATS
};

/* ---------- structures */

struct texture_image
{
    enum texture_image_type type;
    enum texture_image_format format;
    int x;
    int y;
    int width;
    int height;
};

struct texture_data
{
    GLuint id;

    int image_count;
    struct texture_image *images;
};

/* ---------- prototypes */

int texture_new(void);
void texture_delete(int texture_index);
void texture_add_image(int texture_index, struct texture_image *image);
