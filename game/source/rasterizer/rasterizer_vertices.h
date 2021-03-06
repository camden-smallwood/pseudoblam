/*
RASTERIZER_VERTICES.H
    Rasterizer vertex management declarations.
*/

#pragma once
#include <stddef.h>
#include <cglm/cglm.h>
#include <GL/glew.h>

/* ---------- types */

enum vertex_type
{
    _vertex_type_flat,
    _vertex_type_rigid,
    _vertex_type_skinned,
    NUMBER_OF_VERTEX_TYPES
};

struct vertex_flat
{
    vec2 position;
    vec2 texcoord;
};

struct vertex_rigid
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
    vec3 tangent;
    vec3 bitangent;
};

struct vertex_skinned
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
    vec3 tangent;
    vec3 bitangent;
    int node_indices[4];
    float node_weights[4];
};

struct vertex_attribute_definition
{
    GLenum element_type;
    GLint element_count;
    GLboolean normalized;
    const char *name;
    size_t offset;
};

struct vertex_definition
{
    unsigned int size;
    int attribute_count;
    const struct vertex_attribute_definition *attributes;
};

/* ---------- prototypes/RASTERIZER_VERTICES.C */

const struct vertex_definition *vertex_definition_get(enum vertex_type type);
