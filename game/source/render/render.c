/*
RENDER.C
    Main rendering code.
*/

/* ---------- headers */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL_scancode.h>
#include <GL/glew.h>
#include <cglm/cglm.h>

#include "common/common.h"
#include "camera/camera.h"
#include "game/game.h"
#include "models/models.h"
#include "objects/objects.h"
#include "textures/dds.h"

#include "objects/lights.h"
#include "render/render.h"

#include "rasterizer/rasterizer_render_targets.h"
#include "rasterizer/rasterizer_shaders.h"
#include "rasterizer/rasterizer_textures.h"

/* ---------- private prototypes */

static void render_initialize_quad(void);
static void render_quad(void);

static void render_set_lighting_uniforms(int shader_index);
static void render_set_material_uniforms(int shader_index, struct material_data *material);

static void render_object(int shader_index, int object_index);

/* ---------- geometry pass */

enum render_geometry_pass_attachment
{
    _render_geometry_pass_position_texture,
    _render_geometry_pass_velocity_texture,
    _render_geometry_pass_normal_texture,
    _render_geometry_pass_albedo_specular_texture,
    _render_geometry_pass_material_texture,
    _render_geometry_pass_emissive_texture,
    _render_geometry_pass_view_normal_texture,
    _render_geometry_pass_depth_buffer,
    NUMBER_OF_RENDER_GEOMETRY_PASS_ATTACHMENTS
};

struct render_geometry_pass_data
{
    int shader_index;

    struct framebuffer framebuffer;
    struct renderbuffer depth_buffer;

    int position_texture_index;
    int velocity_texture_index;
    int normal_texture_index;
    int albedo_specular_texture_index;
    int material_texture_index;
    int emissive_texture_index;
    int view_normal_texture_index;
};

static void render_initialize_geometry_pass(void);
static void render_resize_geometry_pass(void);
static void render_geometry_pass(void);

/* ---------- depth pass */

enum render_depth_pass_attachment
{
    _render_depth_pass_depth_texture,
    NUMBER_OF_RENDER_DEPTH_PASS_ATTACHMENTS
};

struct render_depth_pass_data
{
    struct framebuffer framebuffer;
    int texture_index;
};

static void render_initialize_depth_pass(void);
static void render_resize_depth_pass(void);
static void render_depth_pass(void);

/* ---------- occlusion pass */

enum render_occlusion_pass_constants
{
    NUMBER_OF_SSAO_KERNEL_SAMPLES = 64,

    SSAO_NOISE_TEXTURE_WIDTH = 4,
    SSAO_NOISE_TEXTURE_HEIGHT = 4,
    NUMBER_OF_SSAO_NOISE_POINTS = SSAO_NOISE_TEXTURE_WIDTH * SSAO_NOISE_TEXTURE_HEIGHT,
};

enum render_occlusion_pass_attachment
{
    _render_occlusion_pass_base_texture,
    _render_occlusion_pass_noise_texture,
    NUMBER_OF_RENDER_OCCLUSION_PASS_ATTACHMENTS
};

struct render_occlusion_pass_data
{
    int shader_index;

    struct framebuffer framebuffer;
    
    int base_texture_index;
    int noise_texture_index;

    vec3 kernel_samples[NUMBER_OF_SSAO_KERNEL_SAMPLES];
    vec3 noise_points[NUMBER_OF_SSAO_NOISE_POINTS];
};

static void render_initialize_occlusion_pass(void);
static void render_resize_occlusion_pass(void);
static void render_occlusion_pass(void);

/* ---------- shadow pass */

enum render_shader_pass_attachment
{
    _render_shader_pass_texture,
    NUMBER_OF_RENDER_SHADOW_PASS_ATTACHMENTS
};

struct render_shadow_pass_data
{
    int shader_index;
    
    int texture_width;
    int texture_height;
    int texture_index;

    struct framebuffer framebuffer;
};

static void render_initialize_shadow_pass(void);
static void render_resize_shadow_pass(void);
static void render_shadow_pass(void);

/* ---------- lighting pass */

enum render_lighting_pass_attachment
{
    _render_lighting_pass_base_texture,
    _render_lighting_pass_hdr_texture,
    NUMBER_OF_RENDER_LIGHTING_PASS_ATTACHMENTS
};

struct render_lighting_pass_data
{
    int shader_index;

    struct framebuffer framebuffer;

    int base_texture_index;
    int hdr_texture_index;

    mat4 light_space_matrix;
};

static void render_initialize_lighting_pass(void);
static void render_resize_lighting_pass(void);
static void render_lighting_pass(void);

/* ---------- transparent pass */

struct render_transparent_pass_data
{
    int shader_index;
    int texture_index;
    struct framebuffer framebuffer;
};

static void render_initialize_transparent_pass(void);
static void render_resize_transparent_pass(void);
static void render_transparent_pass(void);

/* ---------- postprocess pass */

enum render_postprocess_pass_attachment
{
    _render_postprocess_pass_texture,
    NUMBER_OF_RENDER_POSTPROCESS_PASS_ATTACHMENTS
};

struct render_postprocess_pass_data
{
    int texture_index;
    struct framebuffer framebuffer;
};

static void render_initialize_postprocess_pass(void);
static void render_resize_postprocess_pass(void);
static void render_postprocess_pass(void);

/* ---------- blur pass */

enum render_blur_pass_attachment
{
    _render_blur_pass_texture,
    NUMBER_OF_RENDER_BLUR_PASS_ATTACHMENTS
};

struct render_blur_pass_data
{
    int shader_index;
    int texture_index;
    struct framebuffer framebuffer;
};

static void render_initialize_blur_pass(void);
static void render_resize_blur_pass(void);
static void render_blur_pass(void);

/* ---------- hdr pass */

struct render_hdr_pass_data
{
    int shader_index;
    int texture_index;
    struct framebuffer framebuffer;
};

static void render_initialize_hdr_pass(void);
static void render_resize_hdr_pass(void);
static void render_hdr_pass(void);

/* ---------- private variables */

struct
{
    unsigned int flags;

    int screen_width;
    int screen_height;
    
    int sample_count;

    int quad_shader;
    GLuint quad_vertex_array;
    GLuint quad_vertex_buffer;

    struct render_geometry_pass_data geometry_pass;
    struct render_depth_pass_data depth_pass;
    struct render_occlusion_pass_data occlusion_pass;
    struct render_shadow_pass_data shadow_pass;
    struct render_lighting_pass_data lighting_pass;
    struct render_transparent_pass_data transparent_pass;
    struct render_postprocess_pass_data postprocess_pass;
    struct render_blur_pass_data blur_pass;
    struct render_hdr_pass_data hdr_pass;
} static render_globals;

/* ---------- public code */

void render_initialize(void)
{
    memset(&render_globals, 0, sizeof(render_globals));

    render_globals.screen_width = 1280;
    render_globals.screen_height = 720;
    render_globals.sample_count = 4;

    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    render_initialize_quad();
    
    render_initialize_geometry_pass();
    render_initialize_depth_pass();
    render_initialize_occlusion_pass();
    render_initialize_shadow_pass();
    render_initialize_lighting_pass();
    render_initialize_transparent_pass();
    render_initialize_postprocess_pass();
    render_initialize_blur_pass();
    render_initialize_hdr_pass();
}

void render_dispose(void)
{
    // TODO: finish
}

void render_handle_screen_resize(int width, int height)
{
    if (width == render_globals.screen_width && height == render_globals.screen_height)
        return;

    render_globals.screen_width = width;
    render_globals.screen_height = height;

    render_resize_geometry_pass();
    render_resize_depth_pass();
    render_resize_occlusion_pass();
    render_resize_shadow_pass();
    render_resize_lighting_pass();
    render_resize_transparent_pass();
    render_resize_postprocess_pass();
    render_resize_blur_pass();
    render_resize_hdr_pass();
}

void render_load_content(void)
{
    static struct model_iterator iterator;
    model_iterator_new(&iterator);

    while (model_iterator_next(&iterator) != -1)
    {
        for (int mesh_index = 0; mesh_index < iterator.data->mesh_count; mesh_index++)
        {
            struct model_mesh *mesh = iterator.data->meshes + mesh_index;

            if (!mesh->vertex_data)
                continue;

            // TODO: rasterizer mesh
            glGenVertexArrays(1, &mesh->vertex_array);
            glBindVertexArray(mesh->vertex_array);

            const struct vertex_definition *vertex_definition = vertex_definition_get(mesh->vertex_type);

            // TODO: rasterizer vertex buffer
            glGenBuffers(1, &mesh->vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * vertex_definition->size, mesh->vertex_data, GL_STATIC_DRAW);

            // TODO: rasterizer index buffer
            glGenBuffers(1, &mesh->index_buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(int), mesh->indices, GL_STATIC_DRAW);

            // TODO: rasterizer uniform buffer
            glGenBuffers(1, &mesh->uniform_buffer);
            glBindBuffer(GL_UNIFORM_BUFFER, mesh->uniform_buffer);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4) * MAXIMUM_NUMBER_OF_MODEL_NODES, NULL, GL_STATIC_DRAW);

            shader_bind_vertex_attributes(render_globals.geometry_pass.shader_index, mesh->vertex_type);
        }
    }
}

void render_update(float delta_ticks)
{
    render_geometry_pass();
    render_depth_pass();
    render_occlusion_pass();
    render_shadow_pass();
    render_lighting_pass();
    render_transparent_pass();
    render_postprocess_pass();
    render_blur_pass();
    render_hdr_pass();
    render_quad();
}

/* ---------- private code */

static void render_initialize_quad(void)
{
    render_globals.quad_shader = shader_new("../assets/shaders/quad.vs", "../assets/shaders/texture.fs");

    struct vertex_flat quad_vertices[] =
    {
        { .position = { -1.0f, 1.0f }, .texcoord = { 0.0f, 1.0f } },
        { .position = { -1.0f, -1.0f }, .texcoord = { 0.0f, 0.0f } },
        { .position = { 1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f } },
        { .position = { -1.0f, 1.0f }, .texcoord = { 0.0f, 1.0f } },
        { .position = { 1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f } },
        { .position = { 1.0f, 1.0f }, .texcoord = { 1.0f, 1.0f } },
    };

    // TODO: rasterizer mesh
    glGenVertexArrays(1, &render_globals.quad_vertex_array);
    glBindVertexArray(render_globals.quad_vertex_array);
    
    // TODO: rasterizer vertex buffer
    glGenBuffers(1, &render_globals.quad_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, render_globals.quad_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    // TODO: bind the flat vertex attributes to the shader of each render pass
    shader_bind_vertex_attributes(render_globals.quad_shader, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.occlusion_pass.shader_index, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.lighting_pass.shader_index, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.blur_pass.shader_index, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.hdr_pass.shader_index, _vertex_type_flat);
}

static void render_quad(void)
{
    framebuffer_use(NULL);

    // TODO: framebuffer_clear (?)
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(render_globals.quad_shader);
    
    shader_bind_texture(render_globals.quad_shader, render_globals.hdr_pass.texture_index, "quad_texture");

    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.quad_shader);
}

static void render_set_lighting_uniforms(int shader_index)
{
    static struct light_iterator light_iterator;
    light_iterator_new(&light_iterator);

    int light_count = 0;

    while (light_iterator_next(&light_iterator) != -1)
    {
        struct light_data *light = light_iterator.data;

        if (TEST_BIT(light->flags, _light_is_hidden_bit))
            continue;
        
        shader_set_uint_v(shader_index, light->type, "lights[%i].type", light_count);
        shader_set_vec3_v(shader_index, light->position, "lights[%i].position", light_count);
        shader_set_vec3_v(shader_index, light->direction, "lights[%i].direction", light_count);
        shader_set_vec3_v(shader_index, light->diffuse_color, "lights[%i].diffuse_color", light_count);
        shader_set_vec3_v(shader_index, light->ambient_color, "lights[%i].ambient_color", light_count);
        shader_set_vec3_v(shader_index, light->specular_color, "lights[%i].specular_color", light_count);
        shader_set_float_v(shader_index, light->constant, "lights[%i].constant", light_count);
        shader_set_float_v(shader_index, light->linear, "lights[%i].linear", light_count);
        shader_set_float_v(shader_index, light->quadratic, "lights[%i].quadratic", light_count);
        shader_set_float_v(shader_index, cosf(glm_rad(light->inner_cutoff)), "lights[%i].inner_cutoff", light_count);
        shader_set_float_v(shader_index, cosf(glm_rad(light->outer_cutoff)), "lights[%i].outer_cutoff", light_count);

        light_count++;
    }

    shader_set_uint(shader_index, light_count, "light_count");
}

static void render_set_material_uniforms(int shader_index, struct material_data *material)
{
    shader_set_vec3(shader_index, material->base_properties.color_diffuse, "material.diffuse_color");

    shader_set_vec3(shader_index, material->base_properties.color_specular, "material.specular_color");
    shader_set_float(shader_index, material->specular_properties.specular_factor, "material.specular_amount");
    shader_set_float(shader_index, material->specular_properties.glossiness_factor, "material.specular_shininess");

    shader_set_vec3(shader_index, material->base_properties.color_ambient, "material.ambient_color");
    shader_set_float(shader_index, 0.1f, "material.ambient_amount");

    shader_set_float(shader_index, material->base_properties.bump_scaling, "material.bump_scaling");

    for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
    {
        struct material_texture *texture = material->textures + texture_index;
        
        const char *texture_variable_name;

        switch (texture->usage)
        {
        case _material_texture_usage_diffuse:
            texture_variable_name = "material.diffuse_texture";
            break;

        case _material_texture_usage_specular:
            texture_variable_name = "material.specular_texture";
            break;

        case _material_texture_usage_emissive:
            texture_variable_name = "material.emissive_texture";
            break;

        case _material_texture_usage_normals:
            texture_variable_name = "material.normal_texture";
            break;
        
        case _material_texture_usage_opacity:
            texture_variable_name = "material.opacity_texture";
            break;

        default:
            fprintf(stderr, "ERROR: unhandled texture usage - \"%s\"\n", material_texture_usage_to_string(texture->usage));
            exit(EXIT_FAILURE);
        }

        shader_bind_texture(shader_index, texture->index, texture_variable_name);
    }
}

static void render_object(int shader_index, int object_index)
{
    struct object_data *object = object_get_data(object_index);
    
    if (!object)
    {
        return;
    }
    
    struct model_data *model = model_get_data(object->model_index);

    if (!model)
    {
        return;
    }

    mat4 position_matrix;
    glm_mat4_identity(position_matrix);
    glm_translate(position_matrix, object->position);

    mat4 yaw_matrix;
    glm_mat4_identity(yaw_matrix);
    glm_rotate(yaw_matrix, glm_rad(object->rotation[0]), (vec3){1, 0, 0});

    mat4 pitch_matrix;
    glm_mat4_identity(pitch_matrix);
    glm_rotate(pitch_matrix, glm_rad(object->rotation[1]), (vec3){0, 1, 0});

    mat4 roll_matrix;
    glm_mat4_identity(roll_matrix);
    glm_rotate(roll_matrix, glm_rad(object->rotation[2]), (vec3){0, 0, 1});

    mat4 rotation_matrix;
    glm_mat4_identity(rotation_matrix);
    glm_mat4_mul(yaw_matrix, pitch_matrix, rotation_matrix);
    glm_mat4_mul(roll_matrix, rotation_matrix, rotation_matrix);

    mat4 scale_matrix;
    glm_mat4_identity(scale_matrix);
    glm_scale(scale_matrix, object->scale);

    mat4 model_matrix;
    glm_mat4_mul(position_matrix, rotation_matrix, model_matrix);
    glm_mat4_mul(model_matrix, scale_matrix, model_matrix);

    struct camera_data *camera = game_get_player_camera();

    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        shader_use(shader_index);

        shader_set_mat4(shader_index, model_matrix, "model");
        shader_set_mat4(shader_index, camera->view, "view");
        shader_set_mat4(shader_index, camera->projection, "projection");

        for (int node_index = 0; node_index < model->node_count; node_index++)
        {
            shader_set_mat4_v(shader_index, object->animations.node_matrices[node_index], "node_matrices[%i]", node_index);
        }
        
        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;

            if (part->material_index == -1)
                continue;

            struct material_data *material = model->materials + part->material_index;
            render_set_material_uniforms(shader_index, material);

            glDrawElements(GL_TRIANGLES, part->index_count, GL_UNSIGNED_INT, (const void *)(part->index_start * sizeof(int)));

            shader_unbind_textures(shader_index);
        }
    }
}

/* ---------- geometry pass */

static void render_initialize_geometry_pass(void)
{
    render_globals.geometry_pass.shader_index = shader_new("../assets/shaders/model.vs", "../assets/shaders/geometry.fs");
    render_globals.geometry_pass.position_texture_index = -1;
    render_globals.geometry_pass.velocity_texture_index = -1;
    render_globals.geometry_pass.normal_texture_index = -1;
    render_globals.geometry_pass.albedo_specular_texture_index = -1;
    render_globals.geometry_pass.material_texture_index = -1;
    render_globals.geometry_pass.emissive_texture_index = -1;
    render_globals.geometry_pass.view_normal_texture_index = -1;

    render_resize_geometry_pass();
}

static void render_resize_geometry_pass(void)
{
    renderbuffer_dispose(&render_globals.geometry_pass.depth_buffer);
    framebuffer_dispose(&render_globals.geometry_pass.framebuffer);
    framebuffer_initialize(&render_globals.geometry_pass.framebuffer);

    texture_delete(render_globals.geometry_pass.position_texture_index);
    render_globals.geometry_pass.position_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.position_texture_index);

    texture_delete(render_globals.geometry_pass.velocity_texture_index);
    render_globals.geometry_pass.velocity_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.velocity_texture_index);

    texture_delete(render_globals.geometry_pass.normal_texture_index);
    render_globals.geometry_pass.normal_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.normal_texture_index);

    texture_delete(render_globals.geometry_pass.albedo_specular_texture_index);
    render_globals.geometry_pass.albedo_specular_texture_index = texture_new(_texture_type_2d, GL_RGBA32F, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.albedo_specular_texture_index);

    texture_delete(render_globals.geometry_pass.material_texture_index);
    render_globals.geometry_pass.material_texture_index = texture_new(_texture_type_2d, GL_RGBA32F, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.material_texture_index);

    texture_delete(render_globals.geometry_pass.emissive_texture_index);
    render_globals.geometry_pass.emissive_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.emissive_texture_index);

    texture_delete(render_globals.geometry_pass.view_normal_texture_index);
    render_globals.geometry_pass.view_normal_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.view_normal_texture_index);

    renderbuffer_dispose(&render_globals.geometry_pass.depth_buffer);
    renderbuffer_initialize(&render_globals.geometry_pass.depth_buffer, 0, GL_DEPTH_COMPONENT, render_globals.screen_width, render_globals.screen_height);
    framebuffer_attach_renderbuffer(&render_globals.geometry_pass.framebuffer, &render_globals.geometry_pass.depth_buffer);

    framebuffer_build(&render_globals.geometry_pass.framebuffer);
}

static void render_geometry_pass(void)
{
    framebuffer_clear(&render_globals.geometry_pass.framebuffer, 0, 0, render_globals.screen_width, render_globals.screen_height);

    static struct object_iterator iterator;
    object_iterator_new(&iterator);

    while (object_iterator_next(&iterator) != -1)
    {
        render_object(render_globals.geometry_pass.shader_index, iterator.index);
    }
}

/* ---------- depth pass */

static void render_initialize_depth_pass(void)
{
    render_globals.depth_pass.texture_index = -1;
    
    render_resize_depth_pass();
}

static void render_resize_depth_pass(void)
{
    framebuffer_dispose(&render_globals.depth_pass.framebuffer);
    framebuffer_initialize(&render_globals.depth_pass.framebuffer);

    texture_delete(render_globals.depth_pass.texture_index);
    render_globals.depth_pass.texture_index = texture_new(_texture_type_2d, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.depth_pass.framebuffer, render_globals.depth_pass.texture_index);

    framebuffer_build(&render_globals.depth_pass.framebuffer);
}

static void render_depth_pass(void)
{
    framebuffer_clear(&render_globals.depth_pass.framebuffer, 0, 0, render_globals.screen_width, render_globals.screen_height);
    framebuffer_copy(
        &render_globals.geometry_pass.framebuffer, _render_geometry_pass_depth_buffer, 0, 0, render_globals.screen_width, render_globals.screen_height,
        &render_globals.depth_pass.framebuffer, _render_depth_pass_depth_texture, 0, 0, render_globals.screen_width, render_globals.screen_height);
}

/* ---------- occlusion pass */

static void render_initialize_occlusion_pass(void)
{
    render_globals.occlusion_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/ssao.fs");
    render_globals.occlusion_pass.base_texture_index = -1;
    render_globals.occlusion_pass.noise_texture_index = texture_new(_texture_type_2d, GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE, 0, SSAO_NOISE_TEXTURE_WIDTH, SSAO_NOISE_TEXTURE_HEIGHT, 0);

    srand(time(NULL));

    for (int i = 0; i < NUMBER_OF_SSAO_KERNEL_SAMPLES; i++)
    {
        vec3 sample =
        {
            ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f,
            ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f,
            ((float)rand() / (float)RAND_MAX),
        };

        glm_normalize(sample);
        glm_vec3_scale(sample, (float)rand() / (float)RAND_MAX, sample);
        
        float scale = (float)i / (float)NUMBER_OF_SSAO_KERNEL_SAMPLES;
        scale = 0.1f + scale * scale * (1.0f - 0.1f);
        glm_vec3_scale(sample, scale, sample);

        glm_vec3_copy(sample, render_globals.occlusion_pass.kernel_samples[i]);
    }

    for (int i = 0; i < NUMBER_OF_SSAO_NOISE_POINTS; i++)
    {
        vec3 noise =
        {
            ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f,
            ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f,
            0.0f,
        };
        
        glm_vec3_copy(noise, render_globals.occlusion_pass.noise_points[i]);
    }

    render_resize_occlusion_pass();
}

static void render_resize_occlusion_pass(void)
{
    framebuffer_dispose(&render_globals.occlusion_pass.framebuffer);
    framebuffer_initialize(&render_globals.occlusion_pass.framebuffer);
    
    texture_delete(render_globals.occlusion_pass.base_texture_index);
    render_globals.occlusion_pass.base_texture_index = texture_new(_texture_type_2d, GL_RED, GL_RED, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.occlusion_pass.framebuffer, render_globals.occlusion_pass.base_texture_index);
    
    framebuffer_build(&render_globals.occlusion_pass.framebuffer);
}

static void render_occlusion_pass(void)
{
    framebuffer_clear(&render_globals.occlusion_pass.framebuffer, 0, 0, render_globals.screen_width, render_globals.screen_height);

    shader_use(render_globals.occlusion_pass.shader_index);

    shader_bind_texture(render_globals.occlusion_pass.shader_index, render_globals.occlusion_pass.noise_texture_index, "noise_texture");
    shader_bind_texture(render_globals.occlusion_pass.shader_index, render_globals.geometry_pass.view_normal_texture_index, "normal_texture");
    shader_bind_texture(render_globals.occlusion_pass.shader_index, render_globals.depth_pass.texture_index, "depth_texture");
    
    for (int i = 0; i < NUMBER_OF_SSAO_KERNEL_SAMPLES; i++)
    {
        shader_set_vec3_v(render_globals.occlusion_pass.shader_index, render_globals.occlusion_pass.kernel_samples[i], "kernel_samples[%i]", i);
    }

    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.occlusion_pass.shader_index);
}

/* ---------- shadow pass */

static void render_initialize_shadow_pass(void)
{
    framebuffer_initialize(&render_globals.shadow_pass.framebuffer);
    
    render_globals.shadow_pass.texture_width = 1024;
    render_globals.shadow_pass.texture_height = 1024;
    render_globals.shadow_pass.texture_index = texture_new(_texture_type_2d, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, 0, render_globals.shadow_pass.texture_width, render_globals.shadow_pass.texture_height, 0);
    framebuffer_attach_texture(&render_globals.shadow_pass.framebuffer, render_globals.shadow_pass.texture_index);

    framebuffer_build(&render_globals.shadow_pass.framebuffer);
}

static void render_resize_shadow_pass(void)
{
    // TODO: signal shadow texture resize?
}

static void render_shadow_pass(void)
{
    framebuffer_clear(&render_globals.shadow_pass.framebuffer, 0, 0, render_globals.shadow_pass.texture_width, render_globals.shadow_pass.texture_height);
    
    // TODO: render objects from each light's perspective
}

/* ---------- lighting pass */

static void render_initialize_lighting_pass(void)
{
    render_globals.lighting_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/lighting.fs");
    render_globals.lighting_pass.base_texture_index = -1;
    render_globals.lighting_pass.hdr_texture_index = -1;

    render_resize_lighting_pass();
}

static void render_resize_lighting_pass(void)
{
    framebuffer_dispose(&render_globals.lighting_pass.framebuffer);
    framebuffer_initialize(&render_globals.lighting_pass.framebuffer);
    
    texture_delete(render_globals.lighting_pass.base_texture_index);
    render_globals.lighting_pass.base_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.lighting_pass.framebuffer, render_globals.lighting_pass.base_texture_index);

    texture_delete(render_globals.lighting_pass.hdr_texture_index);
    render_globals.lighting_pass.hdr_texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.lighting_pass.framebuffer, render_globals.lighting_pass.hdr_texture_index);

    framebuffer_build(&render_globals.lighting_pass.framebuffer);
}

static void render_lighting_pass(void)
{
    framebuffer_clear(&render_globals.lighting_pass.framebuffer, 0, 0, render_globals.screen_width, render_globals.screen_height);

    shader_use(render_globals.lighting_pass.shader_index);
    
    struct camera_data *camera = game_get_player_camera();
    shader_set_vec3(render_globals.lighting_pass.shader_index, camera->position, "camera_position");
    shader_set_vec3(render_globals.lighting_pass.shader_index, camera->forward, "camera_direction");
    shader_set_mat4(render_globals.lighting_pass.shader_index, camera->view, "view");

    shader_bind_texture(render_globals.lighting_pass.shader_index, render_globals.geometry_pass.position_texture_index, "position_texture");
    shader_bind_texture(render_globals.lighting_pass.shader_index, render_globals.geometry_pass.normal_texture_index, "normal_texture");
    shader_bind_texture(render_globals.lighting_pass.shader_index, render_globals.geometry_pass.albedo_specular_texture_index, "albedo_specular_texture");
    shader_bind_texture(render_globals.lighting_pass.shader_index, render_globals.geometry_pass.material_texture_index, "material_texture");
    shader_bind_texture(render_globals.lighting_pass.shader_index, render_globals.geometry_pass.emissive_texture_index, "emissive_texture");
    shader_bind_texture(render_globals.lighting_pass.shader_index, render_globals.occlusion_pass.base_texture_index, "ssao_texture");
    
    render_set_lighting_uniforms(render_globals.lighting_pass.shader_index);

    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.lighting_pass.shader_index);

    framebuffer_copy(
        &render_globals.lighting_pass.framebuffer, _render_lighting_pass_hdr_texture, 0, 0, render_globals.screen_width, render_globals.screen_height,
        &render_globals.postprocess_pass.framebuffer, _render_postprocess_pass_texture, 0, 0, render_globals.screen_width, render_globals.screen_height);
}

/* ---------- transparent pass */

static void render_initialize_transparent_pass(void)
{
    render_globals.transparent_pass.texture_index = -1;

    render_resize_transparent_pass();
}

static void render_resize_transparent_pass(void)
{
    framebuffer_dispose(&render_globals.transparent_pass.framebuffer);
    framebuffer_initialize(&render_globals.transparent_pass.framebuffer);

    texture_delete(render_globals.transparent_pass.texture_index);
    render_globals.transparent_pass.texture_index = texture_new(_texture_type_2d, GL_RGBA32F, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.transparent_pass.framebuffer, render_globals.transparent_pass.texture_index);

    framebuffer_build(&render_globals.transparent_pass.framebuffer);
}

static void render_transparent_pass(void)
{
    // TODO
}

/* ---------- postprocess pass */

static void render_initialize_postprocess_pass(void)
{
    render_globals.postprocess_pass.texture_index = -1;

    render_resize_postprocess_pass();
}

static void render_resize_postprocess_pass(void)
{
    framebuffer_dispose(&render_globals.postprocess_pass.framebuffer);
    framebuffer_initialize(&render_globals.postprocess_pass.framebuffer);

    texture_delete(render_globals.postprocess_pass.texture_index);
    render_globals.postprocess_pass.texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.postprocess_pass.framebuffer, render_globals.postprocess_pass.texture_index);

    framebuffer_build(&render_globals.postprocess_pass.framebuffer);
}

static void render_postprocess_pass(void)
{
    // TODO
}

/* ---------- blur pass */

static void render_initialize_blur_pass(void)
{
    render_globals.blur_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/blur.fs");
    render_globals.blur_pass.texture_index = -1;

    render_resize_blur_pass();
}

static void render_resize_blur_pass(void)
{
    framebuffer_dispose(&render_globals.blur_pass.framebuffer);
    framebuffer_initialize(&render_globals.blur_pass.framebuffer);

    texture_delete(render_globals.blur_pass.texture_index);
    render_globals.blur_pass.texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.blur_pass.framebuffer, render_globals.blur_pass.texture_index);

    framebuffer_build(&render_globals.blur_pass.framebuffer);
}

static void render_blur_pass(void)
{
    bool blur_horizontal = true;
    const int blur_pass_count = 1;

    for (int i = 0; i < blur_pass_count; i++)
    {
        for (int blur_axis = 0; blur_axis < 2; blur_axis++, blur_horizontal = !blur_horizontal)
        {
            framebuffer_clear(&render_globals.blur_pass.framebuffer, 0, 0, render_globals.screen_width, render_globals.screen_height);

            shader_use(render_globals.blur_pass.shader_index);

            shader_bind_texture(render_globals.blur_pass.shader_index, render_globals.postprocess_pass.texture_index, "blur_texture");
            shader_set_bool(render_globals.blur_pass.shader_index, blur_horizontal, "blur_horizontal");
            
            // TODO: draw as mesh (?)
            glBindVertexArray(render_globals.quad_vertex_array);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            shader_unbind_textures(render_globals.blur_pass.shader_index);

            framebuffer_copy(
                &render_globals.blur_pass.framebuffer, _render_blur_pass_texture, 0, 0, render_globals.screen_width, render_globals.screen_height,
                &render_globals.postprocess_pass.framebuffer, _render_postprocess_pass_texture, 0, 0, render_globals.screen_width, render_globals.screen_height);
        }
    }
}

/* ---------- hdr pass */

static void render_initialize_hdr_pass(void)
{
    render_globals.hdr_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/bloom.fs");
    render_globals.hdr_pass.texture_index = -1;

    render_resize_hdr_pass();
}

static void render_resize_hdr_pass(void)
{
    framebuffer_dispose(&render_globals.hdr_pass.framebuffer);
    framebuffer_initialize(&render_globals.hdr_pass.framebuffer);

    texture_delete(render_globals.hdr_pass.texture_index);
    render_globals.hdr_pass.texture_index = texture_new(_texture_type_2d, GL_RGB32F, GL_RGB, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.hdr_pass.framebuffer, render_globals.hdr_pass.texture_index);
    
    framebuffer_build(&render_globals.hdr_pass.framebuffer);
}

static void render_hdr_pass(void)
{
    framebuffer_clear(&render_globals.hdr_pass.framebuffer, 0, 0, render_globals.screen_width, render_globals.screen_height);

    shader_use(render_globals.hdr_pass.shader_index);
    
    shader_bind_texture(render_globals.hdr_pass.shader_index,render_globals.lighting_pass.base_texture_index, "base_texture");
    shader_bind_texture(render_globals.hdr_pass.shader_index,render_globals.postprocess_pass.texture_index, "hdr_texture");
    shader_set_bool(render_globals.hdr_pass.shader_index, true, "bloom");
    
    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.hdr_pass.shader_index);
}
