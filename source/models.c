#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "models.h"

/* ---------- private constants */

enum
{
    AI_IMPORT_FLAGS = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType,
};

/* ---------- public code */

void import_model(const char *file_path)
{
    const struct aiScene *scene = aiImportFile(file_path, AI_IMPORT_FLAGS);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "ERROR: failed to import \"%s\"\n", file_path);
        exit(EXIT_FAILURE);
    }

    aiReleaseImport(scene);
}

void process_scene_node(const struct aiScene *scene, const struct aiNode *node)
{
    assert(scene);
    assert(node);

    

    for (unsigned int mesh_index = 0; mesh_index < node->mNumMeshes; mesh_index++)
    {
        struct aiMesh *mesh = scene->mMeshes[mesh_index];
        assert(mesh);

        // TODO: build render mesh
    }
}
