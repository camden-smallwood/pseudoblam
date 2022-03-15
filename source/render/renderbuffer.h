#pragma once
#include <GL/glew.h>

/* ---------- types */

struct renderbuffer
{
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
