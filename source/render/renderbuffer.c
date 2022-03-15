#include <assert.h>
#include "common/common.h"
#include "render/renderbuffer.h"

/* ---------- render buffers */

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
