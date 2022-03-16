/*
RASTERIZER_SHADERS.H
    Rasterizer shader management declarations.
*/

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "common/common.h"
#include "rasterizer/rasterizer_shaders.h"

/* ---------- private constants */

enum shader_constants
{
    MAXIMUM_NUMBER_OF_ACTIVE_TEXTURES = 32,
};

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
    if (shader_index == -1)
        return NULL;
    
    assert(shader_index >= 0 && shader_index < shader_globals.shader_count);
    return shader_globals.shaders + shader_index;
}

void shader_use(
    int shader_index)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);

    for (int texture_index = 0; texture_index < MAXIMUM_NUMBER_OF_ACTIVE_TEXTURES; texture_index++)
    {
        if (TEST_BIT(shader->active_textures, texture_index))
        {
            glActiveTexture(GL_TEXTURE0 + texture_index);
            glBindTexture(GL_TEXTURE_2D, shader->textures[texture_index]);
        }
    }

    glUseProgram(shader->program);
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

void shader_set_bool(
    int shader_index,
    bool value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);
    
    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniform1i(location, value);
    }
}

void shader_set_bool_v(
    int shader_index,
    bool value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_bool(shader_index, value, name);

    free(name);
}

void shader_set_int(
    int shader_index,
    int value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);
    
    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniform1i(location, value);
    }
}

void shader_set_int_v(
    int shader_index,
    int value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_int(shader_index, value, name);

    free(name);
}

void shader_set_uint(
    int shader_index,
    unsigned int value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);
    
    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniform1ui(location, value);
    }
}

void shader_set_uint_v(
    int shader_index,
    unsigned int value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_uint(shader_index, value, name);

    free(name);
}

void shader_set_float(
    int shader_index,
    float value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);
    
    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniform1f(location, value);
    }
}

void shader_set_float_v(
    int shader_index,
    float value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_float(shader_index, value, name);

    free(name);
}

void shader_set_vec2(
    int shader_index,
    vec2 value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);
    
    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniform2fv(location, 1, value);
    }
}

void shader_set_vec2_v(
    int shader_index,
    vec2 value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_vec2(shader_index, value, name);

    free(name);
}

void shader_set_vec3(
    int shader_index,
    vec3 value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);

    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniform3fv(location, 1, value);
    }
}

void shader_set_vec3_v(
    int shader_index,
    vec3 value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_vec3(shader_index, value, name);

    free(name);
}

void shader_set_mat4(
    int shader_index,
    mat4 value,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);

    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
    }
}

void shader_set_mat4_v(
    int shader_index,
    mat4 value,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_mat4(shader_index, value, name);

    free(name);
}

void shader_set_texture(
    int shader_index,
    GLuint texture,
    int texture_index,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);

    GLint location = glGetUniformLocation(shader->program, name);

    if (location != -1)
    {
        assert(!TEST_BIT(shader->active_textures, texture_index));
        SET_BIT(shader->active_textures, texture_index, true);

        shader->textures[texture_index] = texture;
        
        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, texture);

        glUniform1i(location, texture_index);
    }
}

void shader_set_texture_v(
    int shader_index,
    GLuint texture,
    int texture_index,
    const char *fmt,
    ...)
{
    va_list va;
    va_start(va, fmt);

    char *name;
    vasprintf(&name, fmt, va);

    va_end(va);
    
    shader_set_texture(shader_index, texture, texture_index, name);

    free(name);
}

int shader_bind_texture(
    int shader_index,
    GLuint texture,
    const char *name)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);

    for (int texture_index = 0; texture_index < MAXIMUM_NUMBER_OF_ACTIVE_TEXTURES; texture_index++)
    {
        if (!TEST_BIT(shader->active_textures, texture_index))
        {
            shader_set_texture(shader_index, texture, texture_index, name);
            return texture_index;
        }
    }

    fprintf(stderr, "ERROR: too many textures bound\n");
    exit(EXIT_FAILURE);
}

void shader_unbind_texture(
    int shader_index,
    int texture_index)
{
    struct shader_data *shader = shader_get_data(shader_index);
    assert(shader);

    if (TEST_BIT(shader->active_textures, texture_index))
    {
        SET_BIT(shader->active_textures, texture_index, false);

        shader->textures[texture_index] = 0;

        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void shader_unbind_textures(
    int shader_index)
{
    for (int texture_index = 0; texture_index < MAXIMUM_NUMBER_OF_ACTIVE_TEXTURES; texture_index++)
    {
        shader_unbind_texture(shader_index, texture_index);
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
