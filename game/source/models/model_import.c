/*
MODEL_IMPORT.C
    3D model importing code.
*/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <assimp/cimport.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>

#include "common/common.h"
#include "models/models.h"
#include "textures/dds.h"

/* ---------- private code */

static inline char *material_get_assimp_string(
    const struct aiMaterial *in_material,
    const char *key,
    int type,
    int index,
    const char *default_value)
{
    struct aiString string;

    if (AI_SUCCESS == aiGetMaterialString(in_material, key, type, index, &string))
        return strndup(string.data, sizeof(string.data));
    
    return strdup(default_value);
}

static inline int material_get_assimp_int(
    const struct aiMaterial *in_material,
    const char *key,
    int type,
    int index,
    int default_value)
{
    int value;

    if (AI_SUCCESS == aiGetMaterialIntegerArray(in_material, key, type, index, &value, NULL))
        return value;
    
    return default_value;
}

static inline void material_get_assimp_flag(
    const struct aiMaterial *in_material,
    const char *key,
    int type,
    int index,
    unsigned int *flags_address,
    int bit_index)
{
    if (material_get_assimp_int(in_material, key, type, index, 0))
        SET_BIT(*flags_address, bit_index, 1);
}

static inline float material_get_assimp_float(
    const struct aiMaterial *in_material,
    const char *key,
    int type,
    int index,
    float default_value)
{
    float value;

    if (AI_SUCCESS == aiGetMaterialFloatArray(in_material, key, type, index, &value, NULL))
        return value;
    
    return default_value;
}

static inline void material_get_assimp_vec3(
    const struct aiMaterial *in_material,
    const char *key,
    int type,
    int index,
    vec3 out_vec3,
    vec3 default_value)
{
    if (AI_SUCCESS != aiGetMaterialFloatArray(in_material, key, type, index, out_vec3, (unsigned int []){3}))
        glm_vec3_copy(default_value, out_vec3);
}

static inline void material_import_assimp_textures(
    const struct aiMaterial *in_material,
    struct material_data *out_material,
    enum aiTextureType texture_type,
    enum material_texture_usage texture_usage)
{
    struct aiString string;

    for (int i = 0, count = aiGetMaterialTextureCount(in_material, texture_type); i < count; i++)
    {
        if (texture_usage == _material_texture_usage_opacity)
        {
            SET_BIT(out_material->flags, _material_has_transparency_bit, true);
        }

        struct material_texture texture =
        {
            .usage = texture_usage,
            .index = -1,
        };

        enum aiTextureMapping mapping;
        unsigned int uvindex;
        ai_real blend = 1.0f;
        enum aiTextureOp op;
        enum aiTextureMapMode mapmode;
        unsigned int flags;

        if (AI_SUCCESS == aiGetMaterialTexture(in_material, texture_type, i, &string, &mapping, &uvindex, &blend, &op, &mapmode, &flags))
        {
            texture.index = dds_import_file_as_texture2d(string.data);
        }
        
        mempush(&out_material->texture_count, (void **)&out_material->textures, &texture, sizeof(texture), realloc);
    }
}

static void material_import_assimp_base_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    out_material->base_properties.name = material_get_assimp_string(in_material, AI_MATKEY_NAME, "");
    
    if (material_get_assimp_int(in_material, AI_MATKEY_TWOSIDED, 0))
        SET_BIT(out_material->base_properties.flags, _material_is_two_sided_bit, 1);
    
    if (material_get_assimp_int(in_material, AI_MATKEY_ENABLE_WIREFRAME, 0))
        SET_BIT(out_material->base_properties.flags, _material_enable_wireframe_bit, 1);
    
    out_material->base_properties.shading_model = material_get_assimp_int(in_material, AI_MATKEY_SHADING_MODEL, _material_shading_model_blinn);
    out_material->base_properties.blending_mode = material_get_assimp_int(in_material, AI_MATKEY_BLEND_FUNC, _material_blending_mode_default);

    out_material->base_properties.opacity = material_get_assimp_float(in_material, AI_MATKEY_OPACITY, 1.0f);
    out_material->base_properties.transparency_factor = material_get_assimp_float(in_material, AI_MATKEY_TRANSPARENCYFACTOR, 1.0f);
    out_material->base_properties.bump_scaling = material_get_assimp_float(in_material, AI_MATKEY_BUMPSCALING, 1.0f);
    out_material->base_properties.shininess = material_get_assimp_float(in_material, AI_MATKEY_SHININESS, 1.0f);
    out_material->base_properties.reflectivity = material_get_assimp_float(in_material, AI_MATKEY_REFLECTIVITY, 1.0f);
    out_material->base_properties.shininess_strength = material_get_assimp_float(in_material, AI_MATKEY_SHININESS_STRENGTH, 1.0f);
    out_material->base_properties.refracti = material_get_assimp_float(in_material, AI_MATKEY_REFRACTI, 1.0f);
    
    material_get_assimp_vec3(in_material, AI_MATKEY_COLOR_DIFFUSE, out_material->base_properties.color_diffuse, (vec3){1, 1, 1});
    material_get_assimp_vec3(in_material, AI_MATKEY_COLOR_AMBIENT, out_material->base_properties.color_ambient, (vec3){1, 1, 1});
    material_get_assimp_vec3(in_material, AI_MATKEY_COLOR_SPECULAR, out_material->base_properties.color_specular, (vec3){1, 1, 1});
    material_get_assimp_vec3(in_material, AI_MATKEY_COLOR_EMISSIVE, out_material->base_properties.color_emissive, (vec3){0, 0, 0});
    material_get_assimp_vec3(in_material, AI_MATKEY_COLOR_TRANSPARENT, out_material->base_properties.color_transparent, (vec3){0, 0, 0});
    material_get_assimp_vec3(in_material, AI_MATKEY_COLOR_REFLECTIVE, out_material->base_properties.color_reflective, (vec3){0, 0, 0});
    
    out_material->base_properties.global_background_image = material_get_assimp_string(in_material, AI_MATKEY_GLOBAL_BACKGROUND_IMAGE, "");
    out_material->base_properties.global_shaderlang = material_get_assimp_string(in_material, AI_MATKEY_GLOBAL_SHADERLANG, "");
    out_material->base_properties.shader_vertex = material_get_assimp_string(in_material, AI_MATKEY_SHADER_VERTEX, "");
    out_material->base_properties.shader_fragment = material_get_assimp_string(in_material, AI_MATKEY_SHADER_FRAGMENT, "");
    out_material->base_properties.shader_geo = material_get_assimp_string(in_material, AI_MATKEY_SHADER_GEO, "");
    out_material->base_properties.shader_tesselation = material_get_assimp_string(in_material, AI_MATKEY_SHADER_TESSELATION, "");
    out_material->base_properties.shader_primitive = material_get_assimp_string(in_material, AI_MATKEY_SHADER_PRIMITIVE, "");
    out_material->base_properties.shader_compute = material_get_assimp_string(in_material, AI_MATKEY_SHADER_COMPUTE, "");
    
    material_import_assimp_textures(in_material, out_material, aiTextureType_DIFFUSE, _material_texture_usage_diffuse);
    material_import_assimp_textures(in_material, out_material, aiTextureType_SPECULAR, _material_texture_usage_specular);
    material_import_assimp_textures(in_material, out_material, aiTextureType_AMBIENT, _material_texture_usage_ambient);
    material_import_assimp_textures(in_material, out_material, aiTextureType_EMISSIVE, _material_texture_usage_emissive);
    material_import_assimp_textures(in_material, out_material, aiTextureType_NORMALS, _material_texture_usage_normals);
    material_import_assimp_textures(in_material, out_material, aiTextureType_HEIGHT, _material_texture_usage_height);
    material_import_assimp_textures(in_material, out_material, aiTextureType_SHININESS, _material_texture_usage_shininess);
    material_import_assimp_textures(in_material, out_material, aiTextureType_OPACITY, _material_texture_usage_opacity);
    material_import_assimp_textures(in_material, out_material, aiTextureType_DISPLACEMENT, _material_texture_usage_displacement);
    material_import_assimp_textures(in_material, out_material, aiTextureType_LIGHTMAP, _material_texture_usage_lightmap);
    material_import_assimp_textures(in_material, out_material, aiTextureType_REFLECTION, _material_texture_usage_reflection);
}

static void material_import_assimp_pbr_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    material_get_assimp_flag(in_material, AI_MATKEY_USE_COLOR_MAP, &out_material->pbr_properties.flags, _material_use_pbr_base_color_texture_bit);
    material_get_assimp_vec3(in_material, AI_MATKEY_BASE_COLOR, out_material->pbr_properties.base_color, (vec3){1, 1, 1});
    material_import_assimp_textures(in_material, out_material, aiTextureType_BASE_COLOR, _material_texture_usage_base_color);

    material_get_assimp_flag(in_material, AI_MATKEY_USE_METALLIC_MAP, &out_material->pbr_properties.flags, _material_use_pbr_metalness_texture_bit);
    out_material->pbr_properties.metallic_factor = material_get_assimp_float(in_material, AI_MATKEY_METALLIC_FACTOR, 0.0f);
    material_import_assimp_textures(in_material, out_material, aiTextureType_METALNESS, _material_texture_usage_metalness);

    material_get_assimp_flag(in_material, AI_MATKEY_USE_ROUGHNESS_MAP, &out_material->pbr_properties.flags, _material_use_pbr_diffuse_roughness_texture_bit);

    out_material->pbr_properties.anisotropy_factor = material_get_assimp_float(in_material, AI_MATKEY_ANISOTROPY_FACTOR, 0.0f);
}

static void material_import_assimp_specular_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    out_material->specular_properties.specular_factor = material_get_assimp_float(in_material, AI_MATKEY_SPECULAR_FACTOR, 0.5f);
    out_material->specular_properties.glossiness_factor = material_get_assimp_float(in_material, AI_MATKEY_GLOSSINESS_FACTOR, 32.0f);
}

static void material_import_assimp_sheen_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    out_material->sheen_properties.color_factor = material_get_assimp_float(in_material, AI_MATKEY_SHEEN_COLOR_FACTOR, 1.0f);
    out_material->sheen_properties.roughness_factor = material_get_assimp_float(in_material, AI_MATKEY_SHEEN_ROUGHNESS_FACTOR, 0.0f);
    
    material_import_assimp_textures(in_material, out_material, aiTextureType_SHEEN, _material_texture_usage_sheen);
}

static void material_import_assimp_clearcoat_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    out_material->clearcoat_properties.clearcoat_factor = material_get_assimp_float(in_material, AI_MATKEY_CLEARCOAT_FACTOR, 1.0f);
    out_material->clearcoat_properties.roughness_factor = material_get_assimp_float(in_material, AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, 0.0f);

    material_import_assimp_textures(in_material, out_material, aiTextureType_CLEARCOAT, _material_texture_usage_clearcoat);
}

static void material_import_assimp_transmission_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    out_material->transmission_properties.transmission_factor = material_get_assimp_float(in_material, AI_MATKEY_TRANSMISSION_FACTOR, 1.0f);
    
    material_import_assimp_textures(in_material, out_material, aiTextureType_TRANSMISSION, _material_texture_usage_transmission);
}

static void material_import_assimp_volume_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    out_material->volume_properties.thickness_factor = material_get_assimp_float(in_material, AI_MATKEY_VOLUME_THICKNESS_FACTOR, 1.0f);
    out_material->volume_properties.attenuation_distance = material_get_assimp_float(in_material, AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, 1.0f);
    
    material_get_assimp_vec3(in_material, AI_MATKEY_VOLUME_ATTENUATION_DISTANCE, out_material->volume_properties.attenuation_color, (vec3){0, 0, 0});
    
    material_import_assimp_textures(in_material, out_material, aiTextureType_TRANSMISSION, _material_texture_usage_transmission);
}

static void material_import_assimp_emissive_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    material_get_assimp_flag(in_material, AI_MATKEY_USE_EMISSIVE_MAP, &out_material->emissive_properties.flags, _material_use_emissive_texture_bit);
    
    out_material->emissive_properties.intensity = material_get_assimp_float(in_material, AI_MATKEY_EMISSIVE_INTENSITY, 1.0f);
}

static void material_import_assimp_ambient_occlussion_properties(
    const struct aiMaterial *in_material,
    struct material_data *out_material)
{
    if (material_get_assimp_int(in_material, AI_MATKEY_USE_AO_MAP, 0))
        SET_BIT(out_material->ambient_occlussion_properties.flags, _material_use_ambient_occlussion_texture_bit, 1);
}

static void model_import_assimp_material(
    const struct aiMaterial *in_material,
    struct model_data *out_model)
{
    struct material_data material;
    memset(&material, 0, sizeof(material));
    
    material_import_assimp_base_properties(in_material, &material);
    material_import_assimp_pbr_properties(in_material, &material);
    material_import_assimp_specular_properties(in_material, &material);
    material_import_assimp_sheen_properties(in_material, &material);
    material_import_assimp_clearcoat_properties(in_material, &material);
    material_import_assimp_transmission_properties(in_material, &material);
    material_import_assimp_volume_properties(in_material, &material);
    material_import_assimp_emissive_properties(in_material, &material);
    material_import_assimp_ambient_occlussion_properties(in_material, &material);

    mempush(&out_model->material_count, (void **)&out_model->materials, &material, sizeof(material), realloc);
}

static void model_import_assimp_animation(
    const struct aiAnimation *in_animation,
    struct model_data *out_model)
{
    struct animation_data animation;
    memset(&animation, 0, sizeof(animation));
    
    const char *animation_name = in_animation->mName.data;

    if (strncmp(animation_name, "Armature|", 9) == 0)
    {
        animation_name += 9; // blender hack
    }

    animation.name = strdup(animation_name);

    animation.duration = in_animation->mDuration;
    animation.ticks_per_second = in_animation->mTicksPerSecond;

    for (unsigned int channel_index = 0; channel_index < in_animation->mNumChannels; channel_index++)
    {
        struct aiNodeAnim *in_channel = in_animation->mChannels[channel_index];

        if (strncmp("Armature", in_channel->mNodeName.data, in_channel->mNodeName.length) == 0)
            continue; // blender hack

        struct animation_channel channel;
        memset(&channel, 0, sizeof(channel));
        
        channel.type = _animation_channel_type_node;
        
        channel.node_index = model_find_node_by_name(out_model, in_channel->mNodeName.data);
        assert(channel.node_index != -1);
        
        for (unsigned int position_key_index = 0; position_key_index < in_channel->mNumPositionKeys; position_key_index++)
        {
            struct aiVectorKey *in_position_key = in_channel->mPositionKeys + position_key_index;

            struct animation_position_key position_key =
            {
                .time = in_position_key->mTime,
                .position =
                {
                    in_position_key->mValue.x,
                    in_position_key->mValue.y,
                    in_position_key->mValue.z
                }
            };

            mempush(&channel.position_key_count, (void **)&channel.position_keys, &position_key, sizeof(position_key), realloc);
        }

        for (unsigned int rotation_key_index = 0; rotation_key_index < in_channel->mNumRotationKeys; rotation_key_index++)
        {
            struct aiQuatKey *in_rotation_key = in_channel->mRotationKeys + rotation_key_index;

            struct animation_rotation_key rotation_key =
            {
                .time = in_rotation_key->mTime,
                .rotation =
                {
                    in_rotation_key->mValue.x,
                    in_rotation_key->mValue.y,
                    in_rotation_key->mValue.z,
                    in_rotation_key->mValue.w,
                }
            };

            mempush(&channel.rotation_key_count, (void **)&channel.rotation_keys, &rotation_key, sizeof(rotation_key), realloc);
        }

        for (unsigned int scaling_key_index = 0; scaling_key_index < in_channel->mNumScalingKeys; scaling_key_index++)
        {
            struct aiVectorKey *in_scaling_key = in_channel->mScalingKeys + scaling_key_index;

            struct animation_scaling_key scaling_key =
            {
                .time = in_scaling_key->mTime,
                .scaling =
                {
                    in_scaling_key->mValue.x,
                    in_scaling_key->mValue.y,
                    in_scaling_key->mValue.z
                }
            };

            mempush(&channel.scaling_key_count, (void **)&channel.scaling_keys, &scaling_key, sizeof(scaling_key), realloc);
        }

        mempush(&animation.channel_count, (void **)&animation.channels, &channel, sizeof(channel), realloc);
    }

    for (unsigned int channel_index = 0; channel_index < in_animation->mNumMorphMeshChannels; channel_index++)
    {
        struct aiMeshAnim *in_channel = in_animation->mMeshChannels[channel_index];

        struct animation_channel channel;
        memset(&channel, 0, sizeof(channel));
        channel.type = _animation_channel_type_mesh;
        channel.mesh_index = -1;

        for (unsigned int mesh_key_index = 0; mesh_key_index < in_channel->mNumKeys; mesh_key_index++)
        {
            struct aiMeshKey *in_mesh_key = in_channel->mKeys + mesh_key_index;

            struct animation_mesh_key mesh_key =
            {
                .time = in_mesh_key->mTime,
                .mesh_index = (int)in_mesh_key->mValue,
            };

            mempush(&channel.mesh_key_count, (void **)&channel.mesh_keys, &mesh_key, sizeof(mesh_key), realloc);
        }

        mempush(&animation.channel_count, (void **)&animation.channels, &channel, sizeof(channel), realloc);
    }

    for (unsigned int channel_index = 0; channel_index < in_animation->mNumMeshChannels; channel_index++)
    {
        struct aiMeshMorphAnim *in_channel = in_animation->mMorphMeshChannels[channel_index];

        struct animation_channel channel;
        memset(&channel, 0, sizeof(channel));
        channel.type = _animation_channel_type_morph;
        channel.mesh_index = -1;

        for (unsigned int morph_key_index = 0; morph_key_index < in_channel->mNumKeys; morph_key_index++)
        {
            struct aiMeshMorphKey *in_morph_key = in_channel->mKeys + morph_key_index;

            struct animation_morph_key morph_key;
            memset(&morph_key, 0, sizeof(morph_key));
            morph_key.time = in_morph_key->mTime;

            for (unsigned int i = 0; i < in_morph_key->mNumValuesAndWeights; i++)
            {
                mempush_multiple(
                    2,
                    &morph_key.count,
                    (void *[]){
                        &morph_key.values,
                        &morph_key.weights
                    },
                    (void *[]){
                        &in_morph_key->mValues[i],
                        &in_morph_key->mWeights[i]
                    },
                    (size_t[]){
                        sizeof(in_morph_key->mValues[i]),
                        sizeof(in_morph_key->mWeights[i])
                    },
                    realloc);
            }

            mempush(&channel.morph_key_count, (void **)&channel.morph_keys, &morph_key, sizeof(morph_key), realloc);
        }

        mempush(&animation.channel_count, (void **)&animation.channels, &channel, sizeof(channel), realloc);
    }

    mempush(&out_model->animation_count, (void **)&out_model->animations, &animation, sizeof(animation), realloc);
}

static void model_import_assimp_mesh(
    // const struct aiScene *in_scene,
    // const struct aiNode *in_node,
    const struct aiMesh *in_mesh,
    struct model_data *out_model,
    struct model_mesh *out_mesh)
{
    if (in_mesh->mName.length && in_mesh->mName.data[0] == '#')
    {
        return;
    }

    struct model_mesh_part part =
    {
        .material_index = in_mesh->mMaterialIndex,
        .vertex_start = out_mesh->vertex_count,
        .vertex_count = 0,
        .index_start = out_mesh->index_count,
        .index_count = 0,
    };

    int node_indices[in_mesh->mNumVertices][4];
    memset(node_indices, -1, sizeof(int) * in_mesh->mNumVertices * 4);

    float node_weights[in_mesh->mNumVertices][4];
    memset(node_weights, 0, sizeof(float) * in_mesh->mNumVertices * 4);

    for (unsigned int bone_index = 0; bone_index < in_mesh->mNumBones; bone_index++)
    {
        struct aiBone *in_bone = in_mesh->mBones[bone_index];

        int node_index = model_find_node_by_name(out_model, in_bone->mName.data);

        if (node_index == -1)
        {
            struct model_node node =
            {
                .name = strdup(in_bone->mName.data),
                .parent_index = -1,
                .first_child_index = -1,
                .next_sibling_index = -1,
                .offset_matrix = GLM_MAT4_IDENTITY_INIT,
                .default_transform = GLM_MAT4_IDENTITY_INIT,
            };

            if (in_bone->mNode)
                glm_mat4_copy((vec4 *)&in_bone->mNode->mTransformation, node.default_transform);
            
            glm_mat4_transpose(node.default_transform);

            glm_mat4_copy((vec4 *)&in_bone->mOffsetMatrix, node.offset_matrix);
            glm_mat4_transpose(node.offset_matrix);
            
            int parent_node_index = -1;
            
            if (in_bone->mNode && in_bone->mNode->mParent != in_bone->mArmature)
                parent_node_index = model_find_node_by_name(out_model, in_bone->mNode->mParent->mName.data);
            
            node_index = model_node_add_child_node(out_model, parent_node_index, &node);
        }

        assert(node_index >= 0 && node_index < out_model->node_count);

        for (unsigned int weight_index = 0; weight_index < in_bone->mNumWeights; weight_index++)
        {
            struct aiVertexWeight *weight = in_bone->mWeights + weight_index;

            for (int i = 0; i < 4; i++)
            {
                if (node_indices[weight->mVertexId][i] == node_index)
                {
                    node_weights[weight->mVertexId][i] = fmaxf(node_weights[weight->mVertexId][i], weight->mWeight);
                    break;
                }
                
                if (node_indices[weight->mVertexId][i] == -1)
                {
                    node_indices[weight->mVertexId][i] = node_index;
                    node_weights[weight->mVertexId][i] = weight->mWeight;
                    break;
                }
            }
        }
    }
    
    for (unsigned int vertex_index = 0; vertex_index < in_mesh->mNumVertices; vertex_index++)
    {
        struct aiVector3D position = in_mesh->mVertices[vertex_index];
        struct aiVector3D normal = in_mesh->mNormals[vertex_index];
        struct aiVector3D texcoord = in_mesh->mTextureCoords[0] ? in_mesh->mTextureCoords[0][vertex_index] : (struct aiVector3D){0, 0, 0};
        struct aiVector3D tangent = in_mesh->mTangents ? in_mesh->mTangents[vertex_index] : (struct aiVector3D){0, 0, 0};
        struct aiVector3D bitangent = in_mesh->mBitangents ? in_mesh->mBitangents[vertex_index] : (struct aiVector3D){0, 0, 0};

        switch (out_mesh->vertex_type)
        {
        case _vertex_type_rigid:
            {
                struct vertex_rigid vertex;
                glm_vec3_copy((vec3){position.x, position.y, position.z}, vertex.position);
                glm_vec3_copy((vec3){normal.x, normal.y, normal.z}, vertex.normal);
                glm_vec2_copy((vec2){texcoord.x, -texcoord.y}, vertex.texcoord);
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
                glm_vec2_copy((vec2){texcoord.x, -texcoord.y}, vertex.texcoord);
                glm_vec3_copy((vec3){tangent.x, tangent.y, tangent.z}, vertex.tangent);
                glm_vec3_copy((vec3){bitangent.x, bitangent.y, bitangent.z}, vertex.bitangent);
                memcpy(vertex.node_indices, node_indices[vertex_index], sizeof(vertex.node_indices));
                memcpy(vertex.node_weights, node_weights[vertex_index], sizeof(vertex.node_weights));
                mempush(&out_mesh->vertex_count, &out_mesh->vertex_data, &vertex, sizeof(vertex), realloc);
            }
            break;
        
        default:
            fprintf(stderr, "ERROR: unhandled vertex type %i\n", out_mesh->vertex_type);
            exit(EXIT_FAILURE);
        }

        part.vertex_count++;
    }

    for (unsigned int face_index = 0; face_index < in_mesh->mNumFaces; face_index++)
    {
        struct aiFace face = in_mesh->mFaces[face_index];

        for (unsigned int index_index = 0; index_index < face.mNumIndices; index_index++)
        {
            int vertex_index = part.vertex_start + face.mIndices[index_index];
            mempush(&out_mesh->index_count, (void **)&out_mesh->indices, &vertex_index, sizeof(int), realloc);
        }

        part.index_count += face.mNumIndices;
    }

    mempush(&out_mesh->part_count, (void **)&out_mesh->parts, &part, sizeof(part), realloc);
}

static void model_import_assimp_node(
    const char *directory_path,
    enum vertex_type vertex_type,
    const struct aiScene *in_scene,
    const struct aiNode *in_node,
    struct model_data *out_model)
{
    struct model_mesh mesh;
    memset(&mesh, 0, sizeof(mesh));

    mesh.vertex_type = vertex_type;

    for (unsigned int mesh_index = 0; mesh_index < in_node->mNumMeshes; mesh_index++)
    {
        struct aiMesh *in_mesh = in_scene->mMeshes[in_node->mMeshes[mesh_index]];

        model_import_assimp_mesh(/*in_scene, in_node,*/ in_mesh, out_model, &mesh);
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

static void model_import_markers_from_assimp_node(
    const char *directory_path,
    enum vertex_type vertex_type,
    const struct aiScene *in_scene,
    const struct aiNode *in_node,
    struct model_data *out_model)
{
    struct model_mesh mesh;
    memset(&mesh, 0, sizeof(mesh));

    mesh.vertex_type = vertex_type;

    for (unsigned int mesh_index = 0; mesh_index < in_node->mNumMeshes; mesh_index++)
    {
        struct aiMesh *in_mesh = in_scene->mMeshes[in_node->mMeshes[mesh_index]];

        if ((in_mesh->mName.length && in_mesh->mName.data[0] == '#') &&
            model_find_marker_by_name(out_model, in_mesh->mName.data + 1) == -1)
        {
            struct model_marker marker;
            memset(&marker, 0, sizeof(marker));

            assert(marker.name = strndup(in_mesh->mName.data + 1, in_mesh->mName.length - 1));
            marker.node_index = model_find_node_by_name(out_model, in_node->mParent->mName.data);

            mat4 marker_matrix;
            glm_mat4_copy((vec4 *)&in_node->mParent->mTransformation, marker_matrix);
            glm_mat4_transpose(marker_matrix);

            vec4 marker_translation;
            mat4 marker_rotation_matrix;
            glm_vec3_copy(*(vec3 *)&marker_translation, marker.position);
            glm_euler_angles(marker_rotation_matrix, marker.rotation);

            // TODO: get position/rotation from marker_transform

            // printf("marker \"%s\":\n"
            //     "\tnode_index: %i (from node \"%s\")\n"
            //     "\tposition: { x: %f, y: %f, z: %f }\n"
            //     "\trotation: { x: %f, y: %f, z: %f }\n",
            //     marker.name,
            //     marker.node_index, in_node->mParent->mName.data,
            //     marker.position[0], marker.position[1], marker.position[2],
            //     marker.rotation[0], marker.rotation[1], marker.rotation[2]);

            mempush(&out_model->marker_count, (void **)&out_model->markers, &marker, sizeof(marker), realloc);
            return;
        }
    }
    
    for (unsigned int child_index = 0; child_index < in_node->mNumChildren; child_index++)
    {
        model_import_markers_from_assimp_node(directory_path, vertex_type, in_scene, in_node->mChildren[child_index], out_model);
    }
}

/* ---------- public code */

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
    model_import_markers_from_assimp_node(directory_path, vertex_type, scene, scene->mRootNode, model);

    for (unsigned int animation_index = 0; animation_index < scene->mNumAnimations; animation_index++)
    {
        struct aiAnimation *animation = scene->mAnimations[animation_index];
        model_import_assimp_animation(animation, model);
    }

    aiReleaseImport(scene);
    free(directory_path);
    
    return model_index;
}
