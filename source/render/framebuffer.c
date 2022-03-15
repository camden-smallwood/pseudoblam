#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/common.h"
#include "render/framebuffer.h"
#include "render/textures.h"

/* -------- public code */

void framebuffer_initialize(
    struct framebuffer *framebuffer)
{
    assert(framebuffer);

    glGenFramebuffers(1, &framebuffer->id);
}

void framebuffer_dispose(
    struct framebuffer *framebuffer)
{
    assert(framebuffer);

    glDeleteFramebuffers(1, &framebuffer->id);
}

void framebuffer_attach_texture(
    struct framebuffer *framebuffer,
    int texture_index)
{
    assert(framebuffer);

    struct texture_data *texture = texture_get_data(texture_index);
    assert(texture);

    switch (texture->type)
    {
    case _texture_type_2d:
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + framebuffer->texture_count,
            GL_TEXTURE_2D,
            texture->id,
            0);
        break;

    case _texture_type_3d:
        glFramebufferTexture3D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0 + framebuffer->texture_count,
            GL_TEXTURE_3D,
            texture->id,
            0,
            0);
        break;

    default:
        fprintf(
            stderr,
            "ERROR: unsupported framebuffer texture type: %s\n",
            texture_type_to_string(texture->type));
        exit(EXIT_FAILURE);
    }

    mempush(
        &framebuffer->texture_count,
        (void **)&framebuffer->texture_indices,
        &texture_index,
        sizeof(texture_index),
        realloc);
}

void framebuffer_attach_renderbuffer(
    struct framebuffer *framebuffer,
    struct renderbuffer *renderbuffer)
{
    assert(framebuffer);
    assert(renderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->id);
    
    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        renderbuffer->id);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    mempush(
        &framebuffer->renderbuffer_count,
        (void **)&framebuffer->renderbuffers,
        renderbuffer,
        sizeof(*renderbuffer),
        realloc);
}
