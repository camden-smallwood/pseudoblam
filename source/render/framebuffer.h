#pragma once
#include <GL/glew.h>
#include "render/renderbuffer.h"

/* -------- types */

enum framebuffer_attachment_type
{
    _framebuffer_attachment_type_texture,
    _framebuffer_attachment_type_depth,
    NUMBER_OF_FRAMEBUFFER_ATTACHMENT_TYPES
};

struct framebuffer_attachment
{
    enum framebuffer_attachment_type type;
    union
    {
        int texture_index;
        struct renderbuffer renderbuffer;
    };
};

struct framebuffer
{
    int attachment_count;
    struct framebuffer_attachment *attachments;

    GLuint id;
};

/* -------- prototypes/FRAMEBUFFER.C */

void framebuffer_initialize(struct framebuffer *framebuffer);
void framebuffer_dispose(struct framebuffer *framebuffer);
void framebuffer_use(struct framebuffer *framebuffer);
void framebuffer_attach_texture(struct framebuffer *framebuffer, int texture_index);
void framebuffer_attach_renderbuffer(struct framebuffer *framebuffer, struct renderbuffer *renderbuffer);
void framebuffer_build(struct framebuffer *framebuffer);
