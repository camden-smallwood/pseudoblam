/*
RASTERIZER_TEXTURES.C
    Texture management and rasterization code.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "common/common.h"
#include "rasterizer/rasterizer_textures.h"

/* ---------- private variables */

struct
{
    int texture_count;
    struct texture_data *textures;
} static texture_globals;

/* ---------- public code */

void textures_initialize(void)
{
    memset(&texture_globals, 0, sizeof(texture_globals));
}

void textures_dispose(void)
{
    for (int i = 0; i < texture_globals.texture_count; i++)
    {
        texture_delete(i);
    }
}

const char *texture_type_to_string(
    enum texture_type type)
{
    switch (type)
    {
    case _texture_type_2d:
        return "2d";

    case _texture_type_3d:
        return "3d";

    default:
        fprintf(stderr, "ERROR: unhandled texture type: %i\n", type);
        exit(EXIT_FAILURE);
    }
}

int texture_allocate(
    enum texture_type type)
{
    struct texture_data texture;
    memset(&texture, 0, sizeof(texture));

    texture.type = type;

    int texture_index = texture_globals.texture_count;

    mempush(
        &texture_globals.texture_count,
        (void **)&texture_globals.textures,
        &texture,
        sizeof(texture),
        realloc);
    
    return texture_index;
}

int texture_new(
    enum texture_type type,
    int internal_format,
    int pixel_format,
    int pixel_type,
    int samples,
    int width,
    int height,
    int depth)
{
    struct texture_data texture;
    memset(&texture, 0, sizeof(texture));

    texture.type = type;
    texture.internal_format = internal_format;
    texture.pixel_format = pixel_format;
    texture.pixel_type = pixel_type;

    glGenTextures(1, &texture.id);

    int texture_index = texture_globals.texture_count;

    mempush(
        &texture_globals.texture_count,
        (void **)&texture_globals.textures,
        &texture,
        sizeof(texture),
        realloc);
    
    texture_resize(texture_index, samples, width, height, depth);
    
    return texture_index;
}

void texture_delete(
    int texture_index)
{
    struct texture_data *texture = texture_get_data(texture_index);
    assert(texture);

    glDeleteTextures(1, &texture->id);
}

struct texture_data *texture_get_data(
    int texture_index)
{
    if (texture_index == -1)
        return NULL;
    
    assert(texture_index >= 0 && texture_index < texture_globals.texture_count);
    return texture_globals.textures + texture_index;
}

int texture_get_target(
    int texture_index)
{
    struct texture_data *texture = texture_get_data(texture_index);
    assert(texture);
    
    switch (texture->type)
    {
    case _texture_type_2d:
        if (texture->samples)
            return GL_TEXTURE_2D_MULTISAMPLE;
        return GL_TEXTURE_2D;

    case _texture_type_3d:
        return GL_TEXTURE_3D;
    
    default:
        fprintf(stderr, "ERROR: unhandled texture type: %i\n", texture->type);
        exit(EXIT_FAILURE);
    }
}

void texture_resize(
    int texture_index,
    int samples,
    int width,
    int height,
    int depth)
{
    struct texture_data *texture = texture_get_data(texture_index);
    assert(texture);

    texture->samples = samples;
    texture->width = width;
    texture->height = height;
    texture->depth = depth;
    
    texture_set_image_data(texture_index, NULL);
}

void texture_set_image_data(
    int texture_index,
    void *data)
{
    struct texture_data *texture = texture_get_data(texture_index);
    GLenum target = texture_get_target(texture_index);

    glBindTexture(target, texture->id);

    switch (texture->type)
    {
    case _texture_type_2d:
        if (texture->samples)
            glTexImage2DMultisample(target, texture->samples, texture->internal_format, texture->width, texture->height, GL_FALSE);
        else
            glTexImage2D(target, 0, texture->internal_format, texture->width, texture->height, 0, texture->pixel_format, texture->pixel_type, data);
        break;
    
    case _texture_type_3d:
        if (texture->samples)
            glTexImage3DMultisample(target, texture->samples, texture->internal_format, texture->width, texture->height, texture->depth, GL_FALSE);
        else
            glTexImage3D(target, 0, texture->internal_format, texture->width, texture->height, texture->depth, 0, texture->pixel_format, texture->pixel_type, data);
        break;
    
    default:
        fprintf(stderr, "ERROR: unhandled texture type: %i\n", texture->type);
        exit(EXIT_FAILURE);
    }

    // TODO: get from texture_data
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(target, 0);
}
