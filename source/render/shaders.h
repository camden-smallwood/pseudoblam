#pragma once
#include <cglm/cglm.h>
#include <GL/glew.h>
#include "render/vertices.h"

/* ---------- structures */

struct shader_data
{
    GLuint program;
};

/* ---------- prototypes/SHADERS.C */

void shaders_initialize(void);
void shaders_dispose(void);

int shader_new(const char *vertex_shader_path, const char *fragment_shader_path);
void shader_delete(int shader_index);

struct shader_data *shader_get_data(int shader_index);

void shader_use(int shader_index);

void shader_bind_vertex_attributes(int shader_index, enum vertex_type vertex_type);

void shader_set_int(int shader_index, const char *name, int value);
void shader_set_int_v(int shader_index, int value, const char *fmt, ...);

void shader_set_uint(int shader_index, const char *name, unsigned int value);
void shader_set_uint_v(int shader_index, unsigned int value, const char *fmt, ...);

void shader_set_float(int shader_index, const char *name, float value);
void shader_set_float_v(int shader_index, float value, const char *fmt, ...);

void shader_set_vec3(int shader_index, const char *name, vec3 value);
void shader_set_vec3_v(int shader_index, vec3 value, const char *fmt, ...);

void shader_set_mat4(int shader_index, const char *name, mat4 value);
void shader_set_mat4_v(int shader_index, mat4 value, const char *fmt, ...);
