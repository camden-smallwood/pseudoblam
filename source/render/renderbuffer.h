#pragma once
#include <GL/glew.h>

/* ---------- types */

enum renderbuffer_flags
{
    _renderbuffer_has_multiple_samples_bit,
    NUMBER_OF_RENDERBUFFER_FLAGS
};

struct renderbuffer
{
    unsigned int flags;

    int samples;
    int width;
    int height;

    GLenum format;
    GLuint id;
};

/* ---------- prototypes/RENDERBUFFER.C */

void renderbuffer_initialize(struct renderbuffer *buffer, int samples, int format, int width, int height);
void renderbuffer_dispose(struct renderbuffer *buffer);
void renderbuffer_resize(struct renderbuffer *buffer, int samples, int width, int height);
