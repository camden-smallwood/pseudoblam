/*
RASTERIZER_RENDER_TARGETS.C
    Rasterizer render targets code.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
#include "rasterizer/rasterizer_render_targets.h"
#include "rasterizer/rasterizer_textures.h"

/* -------- public code */

void renderbuffer_initialize(
    struct renderbuffer *buffer,
    int samples,
    int format,
    int width,
    int height)
{
    assert(buffer);

    buffer->format = (GLenum)format;

    glGenRenderbuffers(1, &buffer->id);

    renderbuffer_resize(buffer, samples, width, height);
}

void renderbuffer_dispose(
    struct renderbuffer *buffer)
{
    assert(buffer);
    glDeleteRenderbuffers(1, &buffer->id);
}

void renderbuffer_resize(
    struct renderbuffer *buffer,
    int samples,
    int width,
    int height)
{
    assert(buffer);

    GLint maximum_samples;
    glGetIntegerv(GL_MAX_SAMPLES, &maximum_samples);
    assert(samples <= maximum_samples);

    buffer->samples = samples;
    buffer->width = width;
    buffer->height = height;

    glGenRenderbuffers(1, &buffer->id);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->id);

    if (samples)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, buffer->format, width, height);
    else
        glRenderbufferStorage(GL_RENDERBUFFER, buffer->format, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void framebuffer_initialize(
    struct framebuffer *framebuffer)
{
    assert(framebuffer);
    memset(framebuffer, 0, sizeof(*framebuffer));

    glGenFramebuffers(1, &framebuffer->id);
}

void framebuffer_dispose(
    struct framebuffer *framebuffer)
{
    assert(framebuffer);

    glDeleteFramebuffers(1, &framebuffer->id);
}

void framebuffer_use(
    struct framebuffer *framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer ? framebuffer->id : 0);
}

void framebuffer_attach_texture(
    struct framebuffer *framebuffer,
    int texture_index)
{
    assert(framebuffer);

    struct framebuffer_attachment attachment =
    {
        .type = _framebuffer_attachment_type_texture,
        .texture_index = texture_index,
    };

    mempush(
        &framebuffer->attachment_count,
        (void **)&framebuffer->attachments,
        &attachment,
        sizeof(attachment),
        realloc);
}

void framebuffer_attach_renderbuffer(
    struct framebuffer *framebuffer,
    struct renderbuffer *renderbuffer)
{
    assert(framebuffer);
    assert(renderbuffer);

    struct framebuffer_attachment attachment =
    {
        .type = _framebuffer_attachment_type_depth,
    };

    memcpy(
        &attachment.renderbuffer,
        renderbuffer,
        sizeof(*renderbuffer));

    mempush(
        &framebuffer->attachment_count,
        (void **)&framebuffer->attachments,
        &attachment,
        sizeof(attachment),
        realloc);
}

void framebuffer_build(
    struct framebuffer *framebuffer)
{
    int color_attachment_count = 0;
    int depth_attachment_count = 0;

    GLenum *attachments = NULL;

    struct renderbuffer *depth_buffer = NULL;

    for (int i = 0; i < framebuffer->attachment_count; i++)
    {
        struct framebuffer_attachment *attachment = framebuffer->attachments + i;

        switch (attachment->type)
        {
        case _framebuffer_attachment_type_texture:
            {
                struct texture_data *texture = texture_get_data(attachment->texture_index);
                assert(texture);

                int attachment_index;
                int attachment_id;

                if (texture->pixel_format == GL_DEPTH_COMPONENT)
                {
                    attachment_index = depth_attachment_count++;
                    attachment_id = GL_DEPTH_ATTACHMENT + attachment_index;
                    printf("attaching depth component %i\n", attachment_index);
                }
                else
                {
                    attachment_index = color_attachment_count++;
                    attachment_id = GL_COLOR_ATTACHMENT0 + attachment_index;
                    printf("attaching color component %i\n", attachment_index);
                }
                
                assert(attachments = realloc(attachments, sizeof(GLenum) * (color_attachment_count + depth_attachment_count)));
                attachments[color_attachment_count + depth_attachment_count - 1] = attachment_id;

                switch (texture->type)
                {
                case _texture_type_2d:
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_id, GL_TEXTURE_2D, texture->id, 0);
                    break;

                case _texture_type_3d:
                    glFramebufferTexture3D(GL_FRAMEBUFFER, attachment_id, GL_TEXTURE_3D, texture->id, 0, 0);
                    break;

                default:
                    fprintf(stderr, "ERROR: unsupported framebuffer texture type: %s\n", texture_type_to_string(texture->type));
                    exit(EXIT_FAILURE);
                }
                break;
            }
        
        case _framebuffer_attachment_type_depth:
            if (depth_buffer)
            {
                fprintf(stderr, "ERROR: multiple depth buffers not supported in framebuffers\n");
                exit(EXIT_FAILURE);
            }
            depth_buffer = &attachment->renderbuffer;
            break;
        
        default:
            fprintf(stderr, "ERROR: unhandled framebuffer attachment type: %i\n", attachment->type);
            exit(EXIT_FAILURE);
        }
    }

    glDrawBuffers(color_attachment_count + depth_attachment_count, attachments);
    
    if (depth_buffer)
    {
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            depth_buffer->id);
    }

    assert(framebuffer);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}
