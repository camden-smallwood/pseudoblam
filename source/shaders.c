#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "glsl_types.h"

struct shader_builder
{
    size_t source_string_size;
    char *source_string;
};

void shader_builder_initialize(
    struct shader_builder *builder)
{
    assert(builder);
    memset(builder, 0, sizeof(*builder));
}

void shader_builder_dispose(
    struct shader_builder *builder)
{
    assert(builder);
    free(builder->source_string);
}

void shader_builder_push_source_line(
    struct shader_builder *builder,
    const char *source_line)
{
    assert(builder);
    assert(source_line);

    size_t source_line_size = strlen(source_line);
    size_t source_line_offset = builder->source_string_size;

    builder->source_string_size += source_line_size + 2;
    assert(source_line_size < builder->source_string_size);

    builder->source_string = realloc(builder->source_string, builder->source_string_size);
    assert(builder->source_string);

    memcpy(builder->source_string + source_line_offset, source_line, source_line_size);
    builder->source_string[source_line_size + 0] = '\n';
    builder->source_string[source_line_size + 1] = '\0';
}

void shader_builder_push_uniform_declaration(
    struct shader_builder *builder,
    enum glsl_type type,
    const char *name)
{
    assert(builder);
    assert(name);

    const char *type_name = glsl_type_to_string(type);
    size_t type_name_length = strlen(type_name);

    size_t name_length = strlen(name);
    assert(name_length);

    // TODO: verify a uniform with the same type and name is not already defined

    char source_line[/*uniform*/7 + /* */1 + type_name_length + /* */1 + name_length + /*;*/1];
    snprintf(source_line, sizeof(source_line), "uniform %s %s;", type_name, name);

    shader_builder_push_source_line(builder, source_line);
}
