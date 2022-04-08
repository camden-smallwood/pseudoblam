/*
RASTERIZER_VERTICES.C
    Rasterizer vertex management code.
*/

#include "rasterizer/rasterizer_vertices.h"

/* ---------- private constants */

static const struct vertex_attribute_definition vertex_flat_attributes[] =
{
    { GL_FLOAT, 2, GL_FALSE, "position", offsetof(struct vertex_flat, position) },
    { GL_FLOAT, 2, GL_FALSE, "texcoord", offsetof(struct vertex_flat, texcoord) },
};

enum
{
    NUMBER_OF_VERTEX_FLAT_ATTRIBUTES =
        sizeof(vertex_flat_attributes) / sizeof(vertex_flat_attributes[0]),
};

static const struct vertex_attribute_definition vertex_rigid_attributes[] =
{
    { GL_FLOAT, 3, GL_FALSE, "position", offsetof(struct vertex_rigid, position) },
    { GL_FLOAT, 3, GL_FALSE, "normal", offsetof(struct vertex_rigid, normal) },
    { GL_FLOAT, 2, GL_FALSE, "texcoord", offsetof(struct vertex_rigid, texcoord) },
    { GL_FLOAT, 3, GL_FALSE, "tangent", offsetof(struct vertex_rigid, tangent) },
    { GL_FLOAT, 3, GL_FALSE, "bitangent", offsetof(struct vertex_rigid, bitangent) },
};

enum
{
    NUMBER_OF_VERTEX_RIGID_ATTRIBUTES =
        sizeof(vertex_rigid_attributes) / sizeof(vertex_rigid_attributes[0]),
};

static const struct vertex_attribute_definition vertex_skinned_attributes[] =
{
    { GL_FLOAT, 3, GL_FALSE, "position", offsetof(struct vertex_skinned, position) },
    { GL_FLOAT, 3, GL_FALSE, "normal", offsetof(struct vertex_skinned, normal) },
    { GL_FLOAT, 2, GL_FALSE, "texcoord", offsetof(struct vertex_skinned, texcoord) },
    { GL_FLOAT, 3, GL_FALSE, "tangent", offsetof(struct vertex_skinned, tangent) },
    { GL_FLOAT, 3, GL_FALSE, "bitangent", offsetof(struct vertex_skinned, bitangent) },
    { GL_INT, 4, GL_FALSE, "node_indices", offsetof(struct vertex_skinned, node_indices) },
    { GL_FLOAT, 4, GL_FALSE, "node_weights", offsetof(struct vertex_skinned, node_weights) },
};

enum
{
    NUMBER_OF_VERTEX_SKINNED_ATTRIBUTES =
        sizeof(vertex_skinned_attributes) / sizeof(vertex_skinned_attributes[0]),
};

static const struct vertex_definition vertex_definitions[NUMBER_OF_VERTEX_TYPES] =
{
    {
        .size = sizeof(struct vertex_flat),
        .attribute_count = NUMBER_OF_VERTEX_FLAT_ATTRIBUTES,
        .attributes = vertex_flat_attributes,
    },
    {
        .size = sizeof(struct vertex_rigid),
        .attribute_count = NUMBER_OF_VERTEX_RIGID_ATTRIBUTES,
        .attributes = vertex_rigid_attributes,
    },
    {
        .size = sizeof(struct vertex_skinned),
        .attribute_count = NUMBER_OF_VERTEX_SKINNED_ATTRIBUTES,
        .attributes = vertex_skinned_attributes,
    },
};

/* ---------- public code */

const struct vertex_definition *vertex_definition_get(enum vertex_type type)
{
    return &vertex_definitions[type];
}
