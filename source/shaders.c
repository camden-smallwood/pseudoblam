#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "common.h"
#include "shaders.h"

/* ---------- private structures */

struct
{
    int shader_count;
    struct shader_data *shaders;
} static shader_globals;

/* ---------- private prototypes */

static GLuint shader_compile_source(
    GLenum shader_type,
    const char *shader_source);

static GLuint shader_import_and_compile_file(
    GLenum shader_type,
    const char *file_path);

/* ---------- public code */

void shaders_initialize(void)
{
    memset(&shader_globals, 0, sizeof(shader_globals));
}

void shaders_dispose(void)
{
    free(shader_globals.shaders);
}

int shader_new(
    const char *vertex_shader_path,
    const char *fragment_shader_path)
{
    GLuint vertex_shader = shader_import_and_compile_file(
        GL_VERTEX_SHADER,
        vertex_shader_path);
    
    GLuint fragment_shader = shader_import_and_compile_file(
        GL_FRAGMENT_SHADER,
        fragment_shader_path);

    struct shader_data shader;
    memset(&shader, 0, sizeof(shader));
    
    shader.program = glCreateProgram();

    glAttachShader(shader.program, vertex_shader);
    glAttachShader(shader.program, fragment_shader);

    glLinkProgram(shader.program);
    glUseProgram(shader.program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    int shader_index = shader_globals.shader_count;

    mempush(
        &shader_globals.shader_count,
        (void **)&shader_globals.shaders,
        &shader,
        sizeof(shader),
        realloc);
    
    return shader_index;
}

void shader_delete(
    int shader_index)
{
    assert(shader_index >= 0 && shader_index < shader_globals.shader_count);
    // TODO
}

struct shader_data *shader_get_data(
    int shader_index)
{
    assert(shader_index >= 0 && shader_index < shader_globals.shader_count);
    return shader_globals.shaders + shader_index;
}

void shader_bind_vertex_attributes(
    int shader_index,
    enum vertex_type vertex_type)
{
    struct shader_data *shader = shader_get_data(shader_index);
    const struct vertex_definition *vertex_definition = vertex_definition_get(vertex_type);

    for (int attribute_index = 0; attribute_index < vertex_definition->attribute_count; attribute_index++)
    {
        const struct vertex_attribute_definition *attribute = vertex_definition->attributes + attribute_index;

        GLuint location = glGetAttribLocation(shader->program, attribute->name);
        glEnableVertexAttribArray(location);

        switch (attribute->element_type)
        {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
        case GL_INT:
        case GL_UNSIGNED_INT:
            glVertexAttribIPointer(
                location,
                attribute->element_count,
                attribute->element_type,
                vertex_definition->size,
                (const void *)attribute->offset);
            break;
        
        case GL_DOUBLE:
            glVertexAttribLPointer(
                location,
                attribute->element_count,
                attribute->element_type,
                vertex_definition->size,
                (const void *)attribute->offset);
            break;
        
        default:
            glVertexAttribPointer(
                location,
                attribute->element_count,
                attribute->element_type,
                attribute->normalized,
                vertex_definition->size,
                (const void *)attribute->offset);
            break;
        }
    }
}

/* ---------- private code */

static GLuint shader_compile_source(
    GLenum shader_type,
    const char *shader_source)
{
    GLuint result = glCreateShader(shader_type);
    glShaderSource(result, 1, &shader_source, NULL);
    glCompileShader(result);

    GLint compile_status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE)
    {
        GLint maximum_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &maximum_log_length);

        GLsizei log_length;
        char log[maximum_log_length];
        glGetShaderInfoLog(result, maximum_log_length, &log_length, log);

        fprintf(stderr, "GLSL compile error: %s\n", log);
    }

    return result;
}

static GLuint shader_import_and_compile_file(
    GLenum shader_type,
    const char *file_path)
{
    FILE *stream = fopen(file_path, "r");

    fseek(stream, 0, SEEK_END);
    size_t file_size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    char file_data[file_size + 1];
    fread(file_data, file_size, 1, stream);
    file_data[file_size] = '\0';

    fclose(stream);

    return shader_compile_source(shader_type, file_data);
}
