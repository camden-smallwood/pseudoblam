#pragma once
#include <GL/glew.h>
#include "render/renderbuffer.h"

/* -------- types */

struct framebuffer
{
    int texture_count;
    int *texture_indices;

    int renderbuffer_count;
    struct renderbuffer *renderbuffers;

    int attachment_count;
    GLenum *attachments;

    GLuint id;
};

/* -------- prototypes/FRAMEBUFFER.C */

void framebuffer_initialize(struct framebuffer *framebuffer);
void framebuffer_dispose(struct framebuffer *framebuffer);
void framebuffer_attach_texture(struct framebuffer *framebuffer, int texture_index);
void framebuffer_attach_renderbuffer(struct framebuffer *framebuffer, struct renderbuffer *renderbuffer);
