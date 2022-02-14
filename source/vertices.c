#include "vertices.h"

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
        sizeof(vertex_rigid_attributes) / sizeof(struct vertex_attribute_definition),
};

static const struct vertex_attribute_definition vertex_skinned_attributes[] =
{
    { GL_FLOAT, 3, GL_FALSE, "position", offsetof(struct vertex_skinned, position) },
    { GL_FLOAT, 3, GL_FALSE, "normal", offsetof(struct vertex_skinned, normal) },
    { GL_FLOAT, 2, GL_FALSE, "texcoord", offsetof(struct vertex_skinned, texcoord) },
    { GL_FLOAT, 3, GL_FALSE, "tangent", offsetof(struct vertex_skinned, tangent) },
    { GL_FLOAT, 3, GL_FALSE, "bitangent", offsetof(struct vertex_skinned, bitangent) },
    { GL_INT, 4, GL_FALSE, "bone_indices", offsetof(struct vertex_skinned, bone_indices) },
    { GL_FLOAT, 4, GL_FALSE, "bone_weights", offsetof(struct vertex_skinned, bone_weights) },
};

enum
{
    NUMBER_OF_VERTEX_SKINNED_ATTRIBUTES =
        sizeof(vertex_skinned_attributes) / sizeof(struct vertex_attribute_definition),
};

static const struct vertex_definition vertex_definitions[NUMBER_OF_VERTEX_TYPES] =
{
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

const struct vertex_definition *vertex_definition_get(enum vertex_type type)
{
    return &vertex_definitions[type];
}

void vertex_type_bind_attributes(enum vertex_type type, GLuint program)
{
    const struct vertex_definition *vertex_definition = &vertex_definitions[type];

    for (int attribute_index = 0; attribute_index < vertex_definition->attribute_count; attribute_index++)
    {
        const struct vertex_attribute_definition *attribute = vertex_definition->attributes + attribute_index;

        GLuint location = glGetAttribLocation(program, attribute->name);
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
