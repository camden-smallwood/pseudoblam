#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <GL/glew.h>
#include "common.h"
#include "models.h"

/* ---------- private prototypes */

static void render_model_process_assimp_node(
    struct render_model *model,
    const char *directory_path,
    const struct aiScene *scene,
    const struct aiNode *node);

static void render_model_process_assimp_metadata(
    struct render_model *model,
    const char *directory_path,
    const struct aiScene *scene,
    const struct aiNode *node,
    const struct aiMetadata *metadata);

static void render_model_process_assimp_mesh(
    struct render_model *model,
    struct render_mesh *render_mesh,
    const char *directory_path,
    const struct aiScene *scene,
    const struct aiNode *node,
    const struct aiMesh *mesh);

/* ---------- public code */

void render_model_load_file(struct render_model *model, const char *file_path)
{
    assert(model);
    assert(file_path);

    //
    // Attempt to load the scene
    //

    const struct aiScene *scene = aiImportFile(
        file_path,
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_SortByPType |
        aiProcess_FindDegenerates |
        aiProcess_FindInvalidData |
        aiProcess_ValidateDataStructure);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "ERROR: failed to import \"%s\"\n", file_path);
        exit(EXIT_FAILURE);
    }

    //
    // Get the path of the folder containing the scene
    //

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

    //
    // Process the root scene node
    //

    memset(model, 0, sizeof(*model));

    render_model_process_assimp_node(model, directory_path, scene, scene->mRootNode);

    //
    // Clean up
    //

    free(directory_path);
    
    aiReleaseImport(scene);
}

/* ---------- private code */

static void render_model_process_assimp_node(
    struct render_model *model,
    const char *directory_path,
    const struct aiScene *scene,
    const struct aiNode *node)
{
    printf("node \"%s\" has:\n"
        "\tparent node: %s\n"
        "\tmesh count: %i\n"
        "\tchild count: %i\n",
        node->mName.data,
        node->mParent ? node->mParent->mName.data : "<none>",
        node->mNumMeshes,
        node->mNumChildren);
    
    render_model_process_assimp_metadata(model, directory_path, scene, node, node->mMetaData);

    struct render_mesh mesh;
    memset(&mesh, 0, sizeof(mesh));

    for (unsigned int mesh_index = 0; mesh_index < node->mNumMeshes; mesh_index++)
    {
        render_model_process_assimp_mesh(model, &mesh, directory_path, scene, node, scene->mMeshes[node->mMeshes[mesh_index]]);
    }
    
    if (mesh.vertices)
    {
        // Create and bind the mesh's vertex array
        glGenVertexArrays(1, &mesh.vertex_array);
        glBindVertexArray(mesh.vertex_array);

        // Create, bind and fill the mesh's vertex buffer
        glGenBuffers(1, &mesh.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertex_count * sizeof(struct render_vertex), mesh.vertices, GL_STATIC_DRAW);

        mempush(&model->mesh_count, (void **)&model->meshes, &mesh, sizeof(mesh), realloc);
    }

    for (unsigned int child_index = 0; child_index < node->mNumChildren; child_index++)
    {
        render_model_process_assimp_node(model, directory_path, scene, node->mChildren[child_index]);
    }
}

static void render_model_process_assimp_metadata(
    struct render_model *model,
    const char *directory_path,
    const struct aiScene *scene,
    const struct aiNode *node,
    const struct aiMetadata *metadata)
{
    if (!metadata)
        return;

    for (unsigned int property_index = 0; property_index < metadata->mNumProperties; property_index++)
    {
        struct aiString *key = metadata->mKeys + property_index;
        struct aiMetadataEntry *value = metadata->mValues + property_index;

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
            render_model_process_assimp_metadata(model, directory_path, scene, node, (const struct aiMetadata *)value->mData);
            break;

        default:
            asprintf(&value_string, "<unknown>");
            break;
        }

        printf("\t%s: %s\n", key->data, value_string);
        free(value_string);
    }
}

static void render_model_process_assimp_mesh(
    struct render_model *render_model,
    struct render_mesh *render_mesh,
    const char *directory_path,
    const struct aiScene *scene,
    const struct aiNode *node,
    const struct aiMesh *mesh)
{
    assert(render_model);
    assert(render_mesh);
    assert(directory_path);
    assert(scene);
    assert(node);
    assert(mesh);

    printf("mesh in node \"%s\" has:\n", node->mName.data);

    struct render_mesh_part part =
    {
        .material_index = -1, // TODO
        .vertex_index = render_mesh->vertex_count,
        .vertex_count = 0,
    };

    for(unsigned int face_index = 0; face_index < mesh->mNumFaces; face_index++)
    {
        struct aiFace face = mesh->mFaces[face_index];

        for(unsigned int index_index = 0; index_index < face.mNumIndices; index_index++)
        {
            int vertex_index = face.mIndices[index_index];

            struct aiVector3D position = mesh->mVertices[vertex_index];
            struct aiVector3D normal = mesh->mNormals[vertex_index];
            struct aiVector3D texcoord = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][vertex_index] : (struct aiVector3D){0, 0, 0};
            struct aiVector3D tangent = mesh->mTangents[vertex_index];
            struct aiVector3D bitangent = mesh->mBitangents[vertex_index];

            struct render_vertex vertex;
            memcpy(&vertex.position, (vec3){position.x, position.y, position.z}, sizeof(vec3));
            memcpy(&vertex.normal, (vec3){normal.x, normal.y, normal.z}, sizeof(vec3));
            memcpy(&vertex.texcoord, (vec2){texcoord.x, texcoord.y}, sizeof(vec2));
            memcpy(&vertex.tangent, (vec3){tangent.x, tangent.y, tangent.z}, sizeof(vec3));
            memcpy(&vertex.bitangent, (vec3){bitangent.x, bitangent.y, bitangent.z}, sizeof(vec3));

            mempush(&render_mesh->vertex_count, (void **)&render_mesh->vertices, &vertex, sizeof(vertex), realloc);
        }

        part.vertex_count += face.mNumIndices;
    }
    
    mempush(&render_mesh->part_count, (void **)&render_mesh->parts, &part, sizeof(part), realloc);

    struct aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    printf("material has %i properties:\n", material->mNumProperties);

    for (unsigned int property_index = 0; property_index < material->mNumProperties; property_index++)
    {
        struct aiMaterialProperty *property = material->mProperties[property_index];
        
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
}
