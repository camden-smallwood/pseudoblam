#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>

#include "common.h"
#include "dds.h"
#include "models.h"

/* ---------- public variables */

struct
{
    int model_count;
    struct model_data *models;
} static model_globals;

/* ---------- private prototypes */

static void model_import_assimp_node(
    const char *directory_path,
    enum vertex_type vertex_type,
    const struct aiScene *in_scene,
    const struct aiNode *in_node,
    struct model_data *out_model);

static void model_import_assimp_metadata(
    const char *directory_path,
    const struct aiScene *in_scene,
    const struct aiNode *in_node,
    const struct aiMetadata *in_metadata,
    struct model_data *out_model);

static void model_import_assimp_mesh(
    enum vertex_type vertex_type,
    const struct aiNode *in_node,
    const struct aiMesh *in_mesh,
    struct model_mesh *out_mesh);

static void model_import_assimp_material(
    const struct aiMaterial *in_material,
    struct model_data *out_model);

/* ---------- public code */

void models_initialize(void)
{
    memset(&model_globals, 0, sizeof(model_globals));
}

void models_dispose(void)
{
    for (int model_index = 0; model_index < model_globals.model_count; model_index++)
    {
        struct model_data *model = model_globals.models + model_index;

        for (int material_index = 0; material_index < model->material_count; material_index++)
        {
            struct material_data *material = model->materials + material_index;

            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                struct material_texture *texture = material->textures + texture_index;
                assert(texture);

                // TODO
            }
        }

        for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
        {
            struct model_mesh *mesh = model->meshes + mesh_index;
            
            free(mesh->vertex_data);
            free(mesh->parts);
        }

        free(model->materials);
        free(model->meshes);
    }

    free(model_globals.models);
}

int model_new(void)
{
    struct model_data model;
    memset(&model, 0, sizeof(model));
    
    int model_index = model_globals.model_count;
    mempush(&model_globals.model_count, (void **)&model_globals.models, &model, sizeof(model), realloc);
    
    return model_index;
}

void model_delete(
    int model_index)
{
    assert(model_index >= 0 && model_index < model_globals.model_count);
    // TODO
}

struct model_data *model_get_data(
    int model_index)
{
    assert(model_index >= 0 && model_index < model_globals.model_count);
    return model_globals.models + model_index;
}

void model_iterator_new(
    struct model_iterator *iterator)
{
    assert(iterator);

    iterator->data = NULL;
    iterator->index = -1;
}

int model_iterator_next(
    struct model_iterator *iterator)
{
    assert(iterator);

    if (++iterator->index >= model_globals.model_count)
    {
        iterator->data = NULL;
        iterator->index = -1;
        return -1;
    }

    int model_index = iterator->index;
    iterator->data = model_globals.models + model_index;
    
    return model_index;
}

int model_import_from_file(
    enum vertex_type vertex_type,
    const char *file_path)
{
    assert(file_path);

    const struct aiScene *scene = aiImportFile(
        file_path,
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_ValidateDataStructure |
        aiProcess_PopulateArmatureData |
        aiProcess_SortByPType |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "ERROR: failed to import \"%s\"\n", file_path);
        return -1;
    }

    char *directory_path;

    const char *separator = strrchr(file_path, '/');
    if (!separator) separator = strrchr(file_path, '\\');
    
    if (separator)
    {
        const char *file_stem = separator + 1;
        int directory_path_length = (int)(file_stem - file_path);
        char directory_path_string[directory_path_length + 1];
        memcpy(directory_path_string, file_path, directory_path_length);
        directory_path_string[directory_path_length] = '\0';

        directory_path = strdup(directory_path_string);
    }
    else
    {
        directory_path = strdup("./");
    }

    int model_index = model_new();
    struct model_data *model = model_get_data(model_index);

    for (unsigned int material_index = 0; material_index < scene->mNumMaterials; material_index++)
    {
        struct aiMaterial *material = scene->mMaterials[material_index];
        model_import_assimp_material(material, model);
    }

    model_import_assimp_node(directory_path, vertex_type, scene, scene->mRootNode, model);

    aiReleaseImport(scene);
    free(directory_path);

    return model_index;
}

/* ---------- private code */

static void model_import_assimp_node(
    const char *directory_path,
    enum vertex_type vertex_type,
    const struct aiScene *in_scene,
    const struct aiNode *in_node,
    struct model_data *out_model)
{
    printf("node \"%s\" has:\n"
        "\tparent node: %s\n"
        "\tmesh count: %i\n"
        "\tchild count: %i\n",
        in_node->mName.data,
        in_node->mParent ? in_node->mParent->mName.data : "<none>",
        in_node->mNumMeshes,
        in_node->mNumChildren);
    
    model_import_assimp_metadata(directory_path, in_scene, in_node, in_node->mMetaData, out_model);

    struct model_mesh mesh;
    memset(&mesh, 0, sizeof(mesh));

    mesh.vertex_type = _vertex_type_rigid;

    for (unsigned int mesh_index = 0; mesh_index < in_node->mNumMeshes; mesh_index++)
    {
        struct aiMesh *in_mesh = in_scene->mMeshes[in_node->mMeshes[mesh_index]];

        model_import_assimp_mesh(
            mesh.vertex_type,
            in_node,
            in_mesh,
            &mesh);
    }
    
    if (mesh.vertex_data)
    {
        mempush(&out_model->mesh_count, (void **)&out_model->meshes, &mesh, sizeof(mesh), realloc);
    }

    for (unsigned int child_index = 0; child_index < in_node->mNumChildren; child_index++)
    {
        model_import_assimp_node(directory_path, vertex_type, in_scene, in_node->mChildren[child_index], out_model);
    }
}

static void model_import_assimp_metadata(
    const char *directory_path,
    const struct aiScene *in_scene,
    const struct aiNode *in_node,
    const struct aiMetadata *in_metadata,
    struct model_data *out_model)
{
    if (!in_metadata)
        return;

    for (unsigned int property_index = 0; property_index < in_metadata->mNumProperties; property_index++)
    {
        struct aiString *key = in_metadata->mKeys + property_index;
        struct aiMetadataEntry *value = in_metadata->mValues + property_index;

        char *value_string = NULL;
        
        switch (value->mType)
        {
        case AI_BOOL:
            asprintf(&value_string, "%s", *(bool *)value->mData ? "true" : "false");
            break;

        case AI_INT32:
            asprintf(&value_string, "%i", *(int32_t *)value->mData);
            break;

        case AI_UINT64:
            asprintf(&value_string, "%llu", *(uint64_t *)value->mData);
            break;

        case AI_FLOAT:
            asprintf(&value_string, "%f", *(float *)value->mData);
            break;

        case AI_DOUBLE:
            asprintf(&value_string, "%lf", *(double *)value->mData);
            break;

        case AI_AISTRING:
            asprintf(&value_string, "\"%s\"", ((struct aiString *)value->mData)->data);
            break;

        case AI_AIVECTOR3D:
            {
                struct aiVector3D *vector = (struct aiVector3D *)value->mData;
                asprintf(&value_string, "{ x: %f, y: %f, z: %f }", vector->x, vector->y, vector->z);
            }
            break;

        case AI_AIMETADATA:
            model_import_assimp_metadata(directory_path, in_scene, in_node, (const struct aiMetadata *)value->mData, out_model);
            break;

        default:
            asprintf(&value_string, "<unknown>");
            break;
        }

        printf("\t%s: %s\n", key->data, value_string);
        free(value_string);
    }
}

static void model_import_assimp_mesh(
    enum vertex_type vertex_type,
    const struct aiNode *in_node,
    const struct aiMesh *in_mesh,
    struct model_mesh *out_mesh)
{
    printf("%s mesh in node \"%s\" has:\n", vertex_type == _vertex_type_rigid ? "rigid" : "skinned", in_node->mName.data);

    printf("\tbones: %i,\n", in_mesh->mNumBones);

    struct model_mesh_part part =
    {
        .material_index = in_mesh->mMaterialIndex,
        .vertex_index = out_mesh->vertex_count,
        .vertex_count = 0,
    };

    for(unsigned int face_index = 0; face_index < in_mesh->mNumFaces; face_index++)
    {
        struct aiFace face = in_mesh->mFaces[face_index];

        for(unsigned int index_index = 0; index_index < face.mNumIndices; index_index++)
        {
            int vertex_index = face.mIndices[index_index];

            struct aiVector3D position = in_mesh->mVertices[vertex_index];
            struct aiVector3D normal = in_mesh->mNormals[vertex_index];
            struct aiVector3D texcoord = in_mesh->mTextureCoords[0] ? in_mesh->mTextureCoords[0][vertex_index] : (struct aiVector3D){0, 0, 0};
            struct aiVector3D tangent = in_mesh->mTangents[vertex_index];
            struct aiVector3D bitangent = in_mesh->mBitangents[vertex_index];

            switch (vertex_type)
            {
            case _vertex_type_rigid:
                {
                    struct vertex_rigid vertex;
                    glm_vec3_copy((vec3){position.x, position.y, position.z}, vertex.position);
                    glm_vec3_copy((vec3){normal.x, normal.y, normal.z}, vertex.normal);
                    glm_vec2_copy((vec2){texcoord.x, texcoord.y}, vertex.texcoord);
                    glm_vec3_copy((vec3){tangent.x, tangent.y, tangent.z}, vertex.tangent);
                    glm_vec3_copy((vec3){bitangent.x, bitangent.y, bitangent.z}, vertex.bitangent);
                    mempush(&out_mesh->vertex_count, &out_mesh->vertex_data, &vertex, sizeof(vertex), realloc);
                }
                break;
            
            case _vertex_type_skinned:
                {
                    struct vertex_skinned vertex;
                    glm_vec3_copy((vec3){position.x, position.y, position.z}, vertex.position);
                    glm_vec3_copy((vec3){normal.x, normal.y, normal.z}, vertex.normal);
                    glm_vec2_copy((vec2){texcoord.x, texcoord.y}, vertex.texcoord);
                    glm_vec3_copy((vec3){tangent.x, tangent.y, tangent.z}, vertex.tangent);
                    glm_vec3_copy((vec3){bitangent.x, bitangent.y, bitangent.z}, vertex.bitangent);
                    memset(vertex.bone_indices, 0, sizeof(vertex.bone_indices));
                    memset(vertex.bone_weights, 0, sizeof(vertex.bone_weights));
                    mempush(&out_mesh->vertex_count, &out_mesh->vertex_data, &vertex, sizeof(vertex), realloc);
                }
                break;
            
            default:
                fprintf(stderr, "ERROR: unhandled vertex type %i\n", vertex_type);
                exit(EXIT_FAILURE);
            }
        }

        part.vertex_count += face.mNumIndices;
    }
    
    mempush(&out_mesh->part_count, (void **)&out_mesh->parts, &part, sizeof(part), realloc);
}

static void model_import_assimp_material(
    const struct aiMaterial *in_material,
    struct model_data *out_model)
{
    printf("material has %i properties:\n", in_material->mNumProperties);

    for (unsigned int property_index = 0; property_index < in_material->mNumProperties; property_index++)
    {
        struct aiMaterialProperty *property = in_material->mProperties[property_index];

        char *value_string = NULL;

        switch (property->mType)
        {
        case aiPTI_Float:
            asprintf(&value_string, "%f", *(float *)property->mData);
            break;

        case aiPTI_Double:
            asprintf(&value_string, "%lf", *(double *)property->mData);
            break;

        case aiPTI_String:
            asprintf(&value_string, "\"%s\"", ((struct aiString *)property->mData)->data);
            break;

        case aiPTI_Integer:
            asprintf(&value_string, "%i", *(int *)property->mData);
            break;

        case aiPTI_Buffer:
            asprintf(&value_string, "[...]");
            break;
        
        default:
            asprintf(&value_string, "<unknown>");
            break;
        }
        
        printf("\t%s: %s\n", property->mKey.data, value_string);
        free(value_string);
    }

    struct material_data material;
    memset(&material, 0, sizeof(material));

    for (int texture_usage = 0; texture_usage < NUMBER_OF_MATERIAL_TEXTURE_USAGES; texture_usage++)
    {
        if (aiGetMaterialTextureCount(in_material, texture_usage))
        {
            struct aiString path;

            if (AI_SUCCESS == aiGetMaterialTexture(in_material, texture_usage, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL))
            {
                struct material_texture texture =
                {
                    .usage = texture_usage,
                    .id = dds_import_file_as_texture2d(path.data),
                };
                
                mempush(&material.texture_count, (void **)&material.textures, &texture, sizeof(texture), realloc);
            }
        }
    }

    mempush(&out_model->material_count, (void **)&out_model->materials, &material, sizeof(material), realloc);
}
