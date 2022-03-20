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
#include "input/input.h"
#include "models/models.h"
#include "objects/objects.h"
#include "textures/dds.h"

#include "render/render_lights.h"
#include "render/render.h"

#include "rasterizer/rasterizer_render_targets.h"
#include "rasterizer/rasterizer_shaders.h"
#include "rasterizer/rasterizer_textures.h"

/* ---------- private constants */

enum render_flags
{
    _render_input_tab_bit,
    _render_input_h_bit,
};

/* ---------- private variables */

struct render_geometry_pass_data
{
    int shader_index;

    struct framebuffer framebuffer;
    struct renderbuffer depth_buffer;

    int position_texture_index;
    int normal_texture_index;
    int albedo_specular_texture_index;
    int material_texture_index;
    int emissive_texture_index;
    int view_normal_texture_index;
};

struct render_depth_pass_data
{
    struct framebuffer framebuffer;
    int texture_index;
};

enum render_occlusion_ssao_constants
{
    NUMBER_OF_SSAO_KERNEL_SAMPLES = 32,

    SSAO_NOISE_TEXTURE_WIDTH = 4,
    SSAO_NOISE_TEXTURE_HEIGHT = 4,
    NUMBER_OF_SSAO_NOISE_POINTS = SSAO_NOISE_TEXTURE_WIDTH * SSAO_NOISE_TEXTURE_HEIGHT,
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

struct render_lighting_pass_data
{
    int shader_index;

    struct framebuffer framebuffer;

    int base_texture_index;
    int hdr_texture_index;

    mat4 light_space_matrix;
};

struct render_transparent_pass_data
{
    int shader_index;
    int texture_index;
    struct framebuffer framebuffer;
};

struct render_postprocess_blur_pass_data
{
    int shader_index;
    int texture_index;
    struct framebuffer framebuffer;
};

struct render_postprocess_hdr_pass_data
{
    int shader_index;
    int texture_index;
    struct framebuffer framebuffer;
};

struct render_postprocess_pass_data
{
    int texture_index;
    struct framebuffer framebuffer;

    struct render_postprocess_blur_pass_data blur_pass;
    struct render_postprocess_hdr_pass_data hdr_pass;
};

struct
{
    unsigned int flags;

    int screen_width;
    int screen_height;
    
    int sample_count;

    struct camera_data camera;

    struct render_geometry_pass_data geometry_pass;
    struct render_depth_pass_data depth_pass;
    struct render_occlusion_pass_data occlusion_pass;
    struct render_lighting_pass_data lighting_pass;
    struct render_transparent_pass_data transparent_pass;
    struct render_postprocess_pass_data postprocess_pass;

    int quad_shader;
    GLuint quad_vertex_array;
    GLuint quad_vertex_buffer;

    int default_diffuse_texture_index;
    int default_specular_texture_index;
    int default_normals_texture_index;
    int default_emissive_texture_index;

    int flashlight_light_index;
    
    int plane_object_index;
    int weapon_object_index;
    int grunt_object_index;
} static render_globals;

/* ---------- private prototypes */

static void render_initialize_gl(void);

static void render_initialize_quad(void);

static void render_initialize_render_passes(void);
static void render_initialize_geometry_pass(void);
static void render_initialize_depth_pass(void);
static void render_initialize_occlusion_pass(void);
static void render_initialize_lighting_pass(void);
static void render_initialize_transparent_pass(void);
static void render_initialize_postprocess_pass(void);
static void render_initialize_postprocess_blur_pass(void);
static void render_initialize_postprocess_hdr_pass(void);

static void render_initialize_scene(void);
static void render_initialize_models(void);
static void render_initialize_objects(void);

static void render_update_input(float delta_ticks);
static void render_update_flashlight(void);

static void render_frame(void);
static void render_geometry_pass(void);
static void render_depth_pass(void);
static void render_occlusion_pass(void);
static void render_lighting_pass(void);
static void render_transparent_pass(void);
static void render_postprocess_pass(void);
static void render_postprocess_hdr_pass(void);
static void render_postprocess_blur_pass(void);
static void render_quad(void);

static void render_set_lighting_uniforms(int shader_index);
static void render_set_material_uniforms(int shader_index, struct material_data *material);

static void render_object(int shader_index, int object_index);

/* ---------- public code */

void render_initialize(void)
{
    memset(&render_globals, 0, sizeof(render_globals));

    /* -------------------------------------------------------------------------------- */

    render_globals.screen_width = 1280;
    render_globals.screen_height = 720;

    render_globals.sample_count = 4;

    /* -------------------------------------------------------------------------------- */

    render_initialize_gl();
    
    /* -------------------------------------------------------------------------------- */

    render_globals.default_diffuse_texture_index = dds_import_file_as_texture2d("../assets/textures/bricks_diffuse.dds");
    render_globals.default_specular_texture_index = dds_import_file_as_texture2d("../assets/textures/white.dds");
    render_globals.default_normals_texture_index = dds_import_file_as_texture2d("../assets/textures/bricks_normal.dds");
    render_globals.default_emissive_texture_index = dds_import_file_as_texture2d("../assets/textures/black.dds");

    /* -------------------------------------------------------------------------------- */

    camera_initialize(&render_globals.camera);

    /* -------------------------------------------------------------------------------- */

    render_initialize_quad();
    render_initialize_render_passes();
    render_initialize_scene();
    render_initialize_models();
    render_initialize_objects();
}

void render_dispose(void)
{
    // TODO: finish
}

void render_handle_screen_resize(int width, int height)
{
    render_globals.screen_width = width;
    render_globals.screen_height = height;
    
    camera_handle_screen_resize(&render_globals.camera, width, height);
    
    // TODO: resize all screen-sized framebuffer textures
}

void render_update(float delta_ticks)
{
    render_update_input(delta_ticks);
    camera_update(&render_globals.camera, delta_ticks);
    render_update_flashlight();

    render_frame();
}

/* ---------- private code */

static void render_initialize_gl(void)
{
    glewExperimental = GL_TRUE;
    glewInit();

    const GLubyte *renderer = glGetString(GL_RENDERER);
    printf("GL Renderer: %s\n", renderer);

    const GLubyte *version = glGetString(GL_VERSION);
    printf("GL Version: %s\n", version);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

static void render_initialize_quad(void)
{
    render_globals.quad_shader = shader_new("../assets/shaders/quad.vs", "../assets/shaders/texture.fs");

    // TODO: initialize as mesh?
    
    struct vertex_flat quad_vertices[] =
    {
        { .position = { -1.0f, 1.0f }, .texcoord = { 0.0f, 1.0f } },
        { .position = { -1.0f, -1.0f }, .texcoord = { 0.0f, 0.0f } },
        { .position = { 1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f } },
        { .position = { -1.0f, 1.0f }, .texcoord = { 0.0f, 1.0f } },
        { .position = { 1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f } },
        { .position = { 1.0f, 1.0f }, .texcoord = { 1.0f, 1.0f } },
    };

    glGenVertexArrays(1, &render_globals.quad_vertex_array);
    glBindVertexArray(render_globals.quad_vertex_array);
    
    glGenBuffers(1, &render_globals.quad_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, render_globals.quad_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    shader_bind_vertex_attributes(render_globals.quad_shader, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.lighting_pass.shader_index, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.postprocess_pass.hdr_pass.shader_index, _vertex_type_flat);
    shader_bind_vertex_attributes(render_globals.postprocess_pass.blur_pass.shader_index, _vertex_type_flat);
}

static void render_initialize_render_passes(void)
{
    render_initialize_geometry_pass();
    render_initialize_depth_pass();
    render_initialize_occlusion_pass();
    render_initialize_lighting_pass();
    render_initialize_transparent_pass();
    render_initialize_postprocess_pass();
}

static void render_initialize_geometry_pass(void)
{
    render_globals.geometry_pass.shader_index = shader_new("../assets/shaders/model.vs", "../assets/shaders/geometry.fs");

    framebuffer_initialize(&render_globals.geometry_pass.framebuffer);
    framebuffer_use(&render_globals.geometry_pass.framebuffer);

    render_globals.geometry_pass.position_texture_index = texture_new(_texture_type_2d, GL_RGB, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.position_texture_index);

    render_globals.geometry_pass.normal_texture_index = texture_new(_texture_type_2d, GL_RGB, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.normal_texture_index);

    render_globals.geometry_pass.albedo_specular_texture_index = texture_new(_texture_type_2d, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.albedo_specular_texture_index);

    render_globals.geometry_pass.material_texture_index = texture_new(_texture_type_2d, GL_RGB, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.material_texture_index);

    render_globals.geometry_pass.emissive_texture_index = texture_new(_texture_type_2d, GL_RGB, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.emissive_texture_index);

    render_globals.geometry_pass.view_normal_texture_index = texture_new(_texture_type_2d, GL_RGB, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.geometry_pass.framebuffer, render_globals.geometry_pass.view_normal_texture_index);

    renderbuffer_initialize(&render_globals.geometry_pass.depth_buffer, 0, GL_DEPTH_COMPONENT, render_globals.screen_width, render_globals.screen_height);
    framebuffer_attach_renderbuffer(&render_globals.geometry_pass.framebuffer, &render_globals.geometry_pass.depth_buffer);

    framebuffer_build(&render_globals.geometry_pass.framebuffer);

    framebuffer_use(NULL);
}

static void render_initialize_depth_pass(void)
{
    framebuffer_initialize(&render_globals.depth_pass.framebuffer);
    framebuffer_use(&render_globals.depth_pass.framebuffer);

    render_globals.depth_pass.texture_index = texture_new(_texture_type_2d, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.depth_pass.framebuffer, render_globals.depth_pass.texture_index);

    framebuffer_build(&render_globals.depth_pass.framebuffer);

    framebuffer_use(NULL);
}

static void render_initialize_occlusion_pass(void)
{
    render_globals.occlusion_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/ssao.fs");

    framebuffer_initialize(&render_globals.occlusion_pass.framebuffer);
    framebuffer_use(&render_globals.occlusion_pass.framebuffer);
    
    render_globals.occlusion_pass.base_texture_index = texture_new(_texture_type_2d, GL_RGB, GL_RGBA, GL_FLOAT, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.occlusion_pass.framebuffer, render_globals.occlusion_pass.base_texture_index);
    
    framebuffer_build(&render_globals.occlusion_pass.framebuffer);

    framebuffer_use(NULL);

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

    render_globals.occlusion_pass.noise_texture_index = texture_new(_texture_type_2d, GL_RGBA32F, GL_RGB, GL_FLOAT, 0, SSAO_NOISE_TEXTURE_WIDTH, SSAO_NOISE_TEXTURE_HEIGHT, 0);
}

static void render_initialize_lighting_pass(void)
{
    render_globals.lighting_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/lighting.fs");

    framebuffer_initialize(&render_globals.lighting_pass.framebuffer);
    framebuffer_use(&render_globals.lighting_pass.framebuffer);
    
    render_globals.lighting_pass.base_texture_index = texture_new(_texture_type_2d, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.lighting_pass.framebuffer, render_globals.lighting_pass.base_texture_index);

    render_globals.lighting_pass.hdr_texture_index = texture_new(_texture_type_2d, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.lighting_pass.framebuffer, render_globals.lighting_pass.hdr_texture_index);

    framebuffer_build(&render_globals.lighting_pass.framebuffer);
    
    framebuffer_use(NULL);
}

static void render_initialize_transparent_pass(void)
{
    framebuffer_initialize(&render_globals.transparent_pass.framebuffer);
    framebuffer_use(&render_globals.transparent_pass.framebuffer);

    render_globals.transparent_pass.texture_index = texture_new(_texture_type_2d, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.transparent_pass.framebuffer, render_globals.transparent_pass.texture_index);

    framebuffer_build(&render_globals.transparent_pass.framebuffer);

    framebuffer_use(NULL);
}

static void render_initialize_postprocess_pass(void)
{
    framebuffer_initialize(&render_globals.postprocess_pass.framebuffer);
    framebuffer_use(&render_globals.postprocess_pass.framebuffer);

    render_globals.postprocess_pass.texture_index = texture_new(_texture_type_2d, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.postprocess_pass.framebuffer, render_globals.postprocess_pass.texture_index);

    framebuffer_build(&render_globals.postprocess_pass.framebuffer);

    framebuffer_use(NULL);

    render_initialize_postprocess_blur_pass();
    render_initialize_postprocess_hdr_pass();
}

static void render_initialize_postprocess_blur_pass(void)
{
    render_globals.postprocess_pass.blur_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/blur.fs");

    framebuffer_initialize(&render_globals.postprocess_pass.blur_pass.framebuffer);
    framebuffer_use(&render_globals.postprocess_pass.blur_pass.framebuffer);

    render_globals.postprocess_pass.blur_pass.texture_index = texture_new(_texture_type_2d, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.postprocess_pass.blur_pass.framebuffer, render_globals.postprocess_pass.blur_pass.texture_index);

    framebuffer_build(&render_globals.postprocess_pass.blur_pass.framebuffer);

    framebuffer_use(NULL);
}

static void render_initialize_postprocess_hdr_pass(void)
{
    render_globals.postprocess_pass.hdr_pass.shader_index = shader_new("../assets/shaders/quad.vs", "../assets/shaders/bloom.fs");

    framebuffer_initialize(&render_globals.postprocess_pass.hdr_pass.framebuffer);
    framebuffer_use(&render_globals.postprocess_pass.hdr_pass.framebuffer);

    render_globals.postprocess_pass.hdr_pass.texture_index = texture_new(_texture_type_2d, GL_RGBA16F, GL_RGBA, GL_UNSIGNED_BYTE, 0, render_globals.screen_width, render_globals.screen_height, 0);
    framebuffer_attach_texture(&render_globals.postprocess_pass.hdr_pass.framebuffer, render_globals.postprocess_pass.hdr_pass.texture_index);
    
    framebuffer_build(&render_globals.postprocess_pass.hdr_pass.framebuffer);

    framebuffer_use(NULL);
}

static void render_initialize_scene(void)
{
    render_globals.plane_object_index = object_new();
    struct object_data *plane_object = object_get_data(render_globals.plane_object_index);
    plane_object->model_index = model_import_from_file(_vertex_type_rigid, "../assets/models/plane.fbx");

    render_globals.weapon_object_index = object_new();
    struct object_data *weapon = object_get_data(render_globals.weapon_object_index);
    glm_vec3_copy((vec3){0.01f, 0.01f, 0.01f}, weapon->scale);
    weapon->model_index = model_import_from_file(_vertex_type_skinned, "../assets/models/assault_rifle.fbx");
    
    render_globals.grunt_object_index = object_new();
    struct object_data *grunt = object_get_data(render_globals.grunt_object_index);
    glm_vec3_copy((vec3){-5, 0, 0}, grunt->position);
    glm_vec3_copy((vec3){0.5f, 0.5f, 0.5f}, grunt->scale);
    grunt->model_index = model_import_from_file(_vertex_type_skinned, "../assets/models/grunt.fbx");

    struct light_data *light;

    render_globals.flashlight_light_index = light_new();
    light = light_get_data(render_globals.flashlight_light_index);
    light->type = _light_type_spot;
    SET_BIT(light->flags, _light_is_hidden_bit, true);
    glm_vec3_copy(render_globals.camera.position, light->position);
    glm_vec3_copy(render_globals.camera.forward, light->direction);
    glm_vec3_copy((vec3){0.8, 0.8, 0.8}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.1f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;
    light->inner_cutoff = 18.5f;
    light->outer_cutoff = 20.0f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){-2.0f, -1.0f, 20.0f}, light->position);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.009f;
    light->quadratic = 0.0032f;
}

static void render_initialize_models(void)
{
    static struct model_iterator iterator;
    model_iterator_new(&iterator);

    while (model_iterator_next(&iterator) != -1)
    {
        if (iterator.data->material_count == 0)
        {
            struct material_data material;
            memset(&material, 0, sizeof(material));
            mempush(&iterator.data->material_count, (void **)&iterator.data->materials, &material, sizeof(material), realloc);
        }

        for (int material_index = 0; material_index < iterator.data->material_count; material_index++)
        {
            struct material_data *material = iterator.data->materials + material_index;

            if (material->texture_count == 0)
            {
                struct material_texture textures[] =
                {
                    {
                        .usage = _material_texture_usage_diffuse,
                        .index = render_globals.default_diffuse_texture_index,
                    },
                    {
                        .usage = _material_texture_usage_specular,
                        .index = render_globals.default_specular_texture_index,
                    },
                    {
                        .usage = _material_texture_usage_normals,
                        .index = render_globals.default_normals_texture_index,
                    },
                    {
                        .usage = _material_texture_usage_emissive,
                        .index = render_globals.default_emissive_texture_index,
                    },
                };

                mempush(&material->texture_count, (void **)&material->textures, &textures[0], sizeof(textures[0]), realloc);
                mempush(&material->texture_count, (void **)&material->textures, &textures[1], sizeof(textures[1]), realloc);
                mempush(&material->texture_count, (void **)&material->textures, &textures[2], sizeof(textures[2]), realloc);
                mempush(&material->texture_count, (void **)&material->textures, &textures[3], sizeof(textures[3]), realloc);
            }

            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                struct material_texture *texture = material->textures + texture_index;

                if (texture->index != -1)
                    continue;
                
                switch (texture->usage)
                {
                case _material_texture_usage_diffuse:
                    texture->index = render_globals.default_diffuse_texture_index;
                    break;

                case _material_texture_usage_specular:
                    texture->index = render_globals.default_specular_texture_index;
                    break;

                case _material_texture_usage_emissive:
                    texture->index = render_globals.default_emissive_texture_index;
                    break;

                case _material_texture_usage_normals:
                    texture->index = render_globals.default_normals_texture_index;
                    break;

                default:
                    fprintf(stderr, "ERROR: unhandled texture usage - \"%s\"\n", material_texture_usage_to_string(texture->usage));
                    exit(EXIT_FAILURE);
                }
            }
        }

        for (int mesh_index = 0; mesh_index < iterator.data->mesh_count; mesh_index++)
        {
            struct model_mesh *mesh = iterator.data->meshes + mesh_index;

            for (int part_index = 0; part_index < mesh->part_count; part_index++)
            {
                struct model_mesh_part *part = mesh->parts + part_index;

                if (part->material_index < 0 || part->material_index >= iterator.data->material_count)
                    part->material_index = 0;
            }

            if (!mesh->vertex_data)
                continue;
            
            const struct vertex_definition *vertex_definition = vertex_definition_get(mesh->vertex_type);

            glGenVertexArrays(1, &mesh->vertex_array);
            glBindVertexArray(mesh->vertex_array);

            glGenBuffers(1, &mesh->vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * vertex_definition->size, mesh->vertex_data, GL_STATIC_DRAW);

            glGenBuffers(1, &mesh->index_buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_count * sizeof(int), mesh->indices, GL_STATIC_DRAW);

            glGenBuffers(1, &mesh->uniform_buffer);
            glBindBuffer(GL_UNIFORM_BUFFER, mesh->uniform_buffer);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4) * MAXIMUM_NUMBER_OF_MODEL_NODES, NULL, GL_STATIC_DRAW);
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            shader_bind_vertex_attributes(render_globals.geometry_pass.shader_index, mesh->vertex_type);
        }
    }
}

static void render_initialize_objects(void)
{
    struct object_iterator iterator;
    object_iterator_new(&iterator);

    while (object_iterator_next(&iterator) != -1)
    {
        object_initialize(iterator.index);

        if (iterator.index == render_globals.weapon_object_index)
        {
            model_set_animation_active(&iterator.data->animations, 0, true);
        }

        if (iterator.index == render_globals.grunt_object_index)
        {
            model_set_animation_active(&iterator.data->animations, 0, true);
        }
    }
}

static void render_update_input(float delta_ticks)
{
    if (input_is_key_down(SDL_SCANCODE_H))
    {
        SET_BIT(render_globals.flags, _render_input_h_bit, true);
    }
    else if (TEST_BIT(render_globals.flags, _render_input_h_bit))
    {
        SET_BIT(render_globals.flags, _render_input_h_bit, false);

        light_set_hidden(
            render_globals.flashlight_light_index,
            !light_is_hidden(render_globals.flashlight_light_index));
    }

    if (input_is_key_down(SDL_SCANCODE_TAB))
    {
        SET_BIT(render_globals.flags, _render_input_tab_bit, true);
    }
    else if (TEST_BIT(render_globals.flags, _render_input_tab_bit))
    {
        SET_BIT(render_globals.flags, _render_input_tab_bit, false);
        
        if (render_globals.camera.movement_speed < 1000.0f)
            render_globals.camera.movement_speed *= 2.0f;
        else
            render_globals.camera.movement_speed = 1.0f;
    }

    ivec2 mouse_motion_int;
    input_get_mouse_motion(&mouse_motion_int[0], &mouse_motion_int[1]);
    
    vec2 mouse_motion =
    {
        (float)-mouse_motion_int[0],
        (float)-mouse_motion_int[1]
    };
    glm_vec2_scale(mouse_motion, 0.01f, mouse_motion);
    glm_vec2_scale(mouse_motion, render_globals.camera.look_sensitivity, mouse_motion);
    glm_vec2_add(render_globals.camera.rotation, mouse_motion, render_globals.camera.rotation);

    vec3 movement = {0.0f, 0.0f, 0.0f};

    // Forwards and backwards camera movement
    if (input_is_key_down(SDL_SCANCODE_W) && !input_is_key_down(SDL_SCANCODE_S))
        glm_vec3_add(movement, render_globals.camera.forward, movement);
    else if (input_is_key_down(SDL_SCANCODE_S) && !input_is_key_down(SDL_SCANCODE_W))
        glm_vec3_sub(movement, render_globals.camera.forward, movement);

    // Horizontal camera movement
    if (input_is_key_down(SDL_SCANCODE_A) && !input_is_key_down(SDL_SCANCODE_D))
        glm_vec3_add(movement, render_globals.camera.right, movement);
    else if (input_is_key_down(SDL_SCANCODE_D) && !input_is_key_down(SDL_SCANCODE_A))
        glm_vec3_sub(movement, render_globals.camera.right, movement);
    
    // Normalize and clamp movement to ground plane
    glm_vec3_normalize(movement);
    movement[2] = 0.0f;

    // Vertical camera movement
    if (input_is_key_down(SDL_SCANCODE_R) && !input_is_key_down(SDL_SCANCODE_F))
        glm_vec3_add(movement, (vec3){0, 0, 1}, movement);
    else if (input_is_key_down(SDL_SCANCODE_F) && !input_is_key_down(SDL_SCANCODE_R))
        glm_vec3_sub(movement, (vec3){0, 0, 1}, movement);

    // Double movement amount if either shift key is down
    if (input_is_key_down(SDL_SCANCODE_LSHIFT) || input_is_key_down(SDL_SCANCODE_RSHIFT))
        glm_vec3_scale(movement, 2.0f, movement);
    
    // Update the view model animations based on the movement amount
    vec3 ground_movement;
    glm_vec3_copy(movement, ground_movement);
    ground_movement[2] = 0.0f;
    float movement_amount = glm_vec3_norm(ground_movement);

    // Scale the movement amount by the camera's movement speed per tick
    glm_vec3_scale(movement, render_globals.camera.movement_speed * delta_ticks, movement);
    
    // Add the movement amount to the camera velocity
    glm_vec3_copy(movement, render_globals.camera.velocity);

    // Update the view model transform
    struct object_data *weapon = object_get_data(render_globals.weapon_object_index);

    if (weapon)
    {
        if (movement_amount == 0.0f)
        {
            // Reset the walking animation when camera movement stops
            model_set_animation_time(&weapon->animations, 0, 0.0f);
        }
        
        // Play the walking animation as fast as the camera is moving
        model_set_animation_speed(&weapon->animations, 0, movement_amount);

        // Move the view model to the camera position + camera velocity
        glm_vec3_copy(render_globals.camera.position, weapon->position);
        glm_vec3_add(weapon->position, render_globals.camera.velocity, weapon->position);

        // Apply the view model position offset
        glm_vec3_add(weapon->position, (vec3){0.0f, 0.0f, -0.015f}, weapon->position);

        // Rotate the view model in the same direction as the camera
        glm_vec3_copy((vec3){0.0f, -render_globals.camera.rotation[1], render_globals.camera.rotation[0]}, weapon->rotation);
    }
}

static void render_update_flashlight(void)
{
    struct light_data *light = light_get_data(render_globals.flashlight_light_index);
    
    if (!light)
    {
        return;
    }

    // Move the light to the camera position
    glm_vec3_copy(render_globals.camera.position, light->position);

    // Check if the weapon is available
    struct object_data *weapon = object_get_data(render_globals.weapon_object_index);

    vec3 attachment_offset = GLM_VEC3_ZERO_INIT;
    
    if (weapon)
    {
        // Check if the weapon model is available
        struct model_data *model = model_get_data(weapon->model_index);

        if (model)
        {
            // Check if the weapon model has a flashlight marker
            int flashlight_marker_index = model_find_marker_by_name(model, "flashlight.001");

            if (flashlight_marker_index != -1)
            {
                struct model_marker *marker = model->markers + flashlight_marker_index;

                if (marker->node_index != -1)
                {
                    mat4 node_matrix;
                    glm_mat4_copy(weapon->animations.states[0].node_matrices[marker->node_index], node_matrix);

                    vec4 node_translation;
                    mat4 node_rotation_matrix;
                    vec3 node_position;
                    vec3 node_rotation;
                    vec3 node_scale;
                    glm_decompose(node_matrix, node_translation, node_rotation_matrix, node_scale);
                    glm_vec3_copy(*(vec3 *)&node_translation, node_position);
                    glm_euler_angles(node_rotation_matrix, node_rotation);

                    vec3 node_offset = { node_position[0], node_position[2], node_position[1] };
                    glm_vec3_mul(node_offset, weapon->scale, node_offset);
                    glm_vec3_add(attachment_offset, node_offset, attachment_offset);
                }

                vec3 marker_offset = { marker->position[0], marker->position[2], marker->position[1] };
                glm_vec3_mul(marker_offset, weapon->scale, marker_offset);

                glm_vec3_add(attachment_offset, marker_offset, attachment_offset);
            }
        }
    }

    // TODO: project the offset in the camera direction...
    glm_vec3_add(light->position, attachment_offset, light->position);

    glm_vec3_copy(render_globals.camera.forward, light->direction);
}

static void render_frame(void)
{
    render_geometry_pass();
    render_depth_pass();
    render_occlusion_pass();
    render_lighting_pass();
    render_transparent_pass();
    render_postprocess_pass();
    render_quad();
}

static void render_geometry_pass(void)
{
    // TODO: framebuffer_clear
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glEnable(GL_DEPTH_TEST);
    framebuffer_use(&render_globals.geometry_pass.framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static struct object_iterator iterator;
    object_iterator_new(&iterator);

    // render world objects
    while (object_iterator_next(&iterator) != -1)
    {
        // TODO: skip first person objects:
        // if (iterator.index == render_globals.weapon_object_index)
        //     continue;
        
        render_object(render_globals.geometry_pass.shader_index, iterator.index);
    }

    // TODO: render first person models

    // if (render_globals.weapon_model_index != -1)
    // {
    //     int model_index = render_globals.weapon_model_index;

    //     mat4 model_matrix;
    //     glm_mat4_identity(model_matrix);
    
    //     mat4 inverted_view;
    //     glm_mat4_inv(render_globals.camera.view, inverted_view);
    //     glm_mat4_mul(inverted_view, model_matrix, model_matrix);

    //     render_object(render_globals.geometry_pass.shader_index, model_index, model_matrix);
    // }

    framebuffer_use(NULL);
}

static void render_depth_pass(void)
{
    // TODO: framebuffer_clear
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glEnable(GL_DEPTH_TEST);

    // TODO: framebuffer_copy
    glBindFramebuffer(GL_READ_FRAMEBUFFER, render_globals.geometry_pass.framebuffer.id);
    glReadBuffer(GL_DEPTH_ATTACHMENT);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_globals.depth_pass.framebuffer.id);
    glDrawBuffer(GL_DEPTH_ATTACHMENT);
    glBlitFramebuffer(
        0, 0, render_globals.screen_width, render_globals.screen_height,
        0, 0, render_globals.screen_width, render_globals.screen_height,
        GL_DEPTH_BUFFER_BIT,
        GL_NEAREST);
    
    framebuffer_use(NULL);
}

static void render_occlusion_pass(void)
{
    // TODO: framebuffer_clear
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);
    framebuffer_use(&render_globals.occlusion_pass.framebuffer);

    shader_use(render_globals.occlusion_pass.shader_index);

    shader_bind_texture(render_globals.occlusion_pass.shader_index, render_globals.occlusion_pass.noise_texture_index, "noise_texture");
    shader_bind_texture(render_globals.occlusion_pass.shader_index, render_globals.geometry_pass.view_normal_texture_index, "normal_texture");
    shader_bind_texture(render_globals.occlusion_pass.shader_index, render_globals.depth_pass.texture_index, "depth_texture");
    
    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.occlusion_pass.shader_index);

    framebuffer_use(NULL);
}

static void render_lighting_pass(void)
{
    // TODO: framebuffer_clear
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);
    framebuffer_use(&render_globals.lighting_pass.framebuffer);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(render_globals.lighting_pass.shader_index);
    
    shader_set_vec3(render_globals.lighting_pass.shader_index, render_globals.camera.position, "camera_position");
    shader_set_vec3(render_globals.lighting_pass.shader_index, render_globals.camera.forward, "camera_direction");
    shader_set_mat4(render_globals.lighting_pass.shader_index, render_globals.camera.view, "view");

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

    // TODO: framebuffer_copy
    glBindFramebuffer(GL_READ_FRAMEBUFFER, render_globals.lighting_pass.framebuffer.id);
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_globals.postprocess_pass.framebuffer.id);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(
        0, 0, render_globals.screen_width, render_globals.screen_height,
        0, 0, render_globals.screen_width, render_globals.screen_height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);
    
    framebuffer_use(NULL);
}

static void render_transparent_pass(void)
{
    // TODO
}

static void render_postprocess_pass(void)
{
    render_postprocess_blur_pass();
    render_postprocess_hdr_pass();
}

static void render_postprocess_blur_pass(void)
{
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);

    bool blur_horizontal = true;
    const int blur_pass_count = 2;

    for (int i = 0; i < blur_pass_count; blur_horizontal = !blur_horizontal, i++)
    {
        framebuffer_use(&render_globals.postprocess_pass.blur_pass.framebuffer);
        // TODO: framebuffer_clear
        glClear(GL_COLOR_BUFFER_BIT);

        shader_use(render_globals.postprocess_pass.blur_pass.shader_index);

        shader_bind_texture(render_globals.postprocess_pass.blur_pass.shader_index,render_globals.postprocess_pass.texture_index, "blur_texture");
        shader_set_bool(render_globals.postprocess_pass.blur_pass.shader_index, blur_horizontal, "blur_horizontal");
        
        // TODO: draw as mesh (?)
        glBindVertexArray(render_globals.quad_vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        shader_unbind_textures(render_globals.postprocess_pass.blur_pass.shader_index);

        // TODO: framebuffer_copy
        glBindFramebuffer(GL_READ_FRAMEBUFFER, render_globals.postprocess_pass.blur_pass.framebuffer.id);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_globals.postprocess_pass.framebuffer.id);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);

        glBlitFramebuffer(
            0, 0, render_globals.screen_width, render_globals.screen_height,
            0, 0, render_globals.screen_width, render_globals.screen_height,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST);
        
        framebuffer_use(NULL);
    }
}

static void render_postprocess_hdr_pass(void)
{
    // TODO: framebuffer_clear
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);
    framebuffer_use(&render_globals.postprocess_pass.hdr_pass.framebuffer);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(render_globals.postprocess_pass.hdr_pass.shader_index);
    
    shader_bind_texture(render_globals.postprocess_pass.hdr_pass.shader_index,render_globals.lighting_pass.base_texture_index, "base_texture");
    shader_bind_texture(render_globals.postprocess_pass.hdr_pass.shader_index,render_globals.postprocess_pass.texture_index, "hdr_texture");
    shader_set_bool(render_globals.postprocess_pass.hdr_pass.shader_index, true, "bloom");
    
    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.postprocess_pass.hdr_pass.shader_index);

    framebuffer_use(NULL);
}

static void render_quad(void)
{
    // TODO: framebuffer_clear (?)
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(render_globals.quad_shader);
    
    shader_bind_texture(render_globals.quad_shader, render_globals.postprocess_pass.hdr_pass.texture_index, "quad_texture");

    // TODO: draw as mesh (?)
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.quad_shader);
}

static void render_set_lighting_uniforms(int shader_index)
{
    static struct light_iterator light_iterator;
    light_iterator_new(&light_iterator);

    int light_counts[NUMBER_OF_LIGHT_TYPES];
    memset(light_counts, 0, sizeof(light_counts));

    while (light_iterator_next(&light_iterator) != -1)
    {
        struct light_data *light = light_iterator.data;

        if (TEST_BIT(light->flags, _light_is_hidden_bit))
            continue;

        const char *lights_array_name;

        switch (light->type)
        {
        case _light_type_directional:
            lights_array_name = "directional_lights";
            break;
        
        case _light_type_point:
            lights_array_name = "point_lights";
            break;
        
        case _light_type_spot:
            lights_array_name = "spot_lights";
            break;
        
        default:
            fprintf(stderr, "ERROR: unhandled light type %i\n", light->type);
            exit(EXIT_FAILURE);
        }

        shader_set_vec3_v(shader_index, light->position, "%s[%i].position", lights_array_name, light_counts[light->type]);
        shader_set_vec3_v(shader_index, light->diffuse_color, "%s[%i].diffuse_color", lights_array_name, light_counts[light->type]);
        shader_set_vec3_v(shader_index, light->ambient_color, "%s[%i].ambient_color", lights_array_name, light_counts[light->type]);
        shader_set_vec3_v(shader_index, light->specular_color, "%s[%i].specular_color", lights_array_name, light_counts[light->type]);

        if (light->type == _light_type_directional || light->type == _light_type_spot)
        {
            shader_set_vec3_v(shader_index, light->direction, "%s[%i].direction", lights_array_name, light_counts[light->type]);
        }

        if (light->type == _light_type_point || light->type == _light_type_spot)
        {
            shader_set_float_v(shader_index, light->constant, "%s[%i].constant", lights_array_name, light_counts[light->type]);
            shader_set_float_v(shader_index, light->linear, "%s[%i].linear", lights_array_name, light_counts[light->type]);
            shader_set_float_v(shader_index, light->quadratic, "%s[%i].quadratic", lights_array_name, light_counts[light->type]);
        }

        if (light->type == _light_type_spot)
        {
            shader_set_float_v(shader_index, cosf(glm_rad(light->inner_cutoff)), "%s[%i].inner_cutoff", lights_array_name, light_counts[light->type]);
            shader_set_float_v(shader_index, cosf(glm_rad(light->outer_cutoff)), "%s[%i].outer_cutoff", lights_array_name, light_counts[light->type]);
        }

        light_counts[light->type]++;
    }

    shader_set_uint(shader_index, light_counts[_light_type_directional], "directional_light_count");
    shader_set_uint(shader_index, light_counts[_light_type_point], "point_light_count");
    shader_set_uint(shader_index, light_counts[_light_type_spot], "spot_light_count");
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

    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        shader_use(shader_index);

        shader_set_mat4(shader_index, model_matrix, "model");
        shader_set_mat4(shader_index, render_globals.camera.view, "view");
        shader_set_mat4(shader_index, render_globals.camera.projection, "projection");

        for (int node_index = 0; node_index < model->node_count; node_index++)
        {
            struct model_node *node = model->nodes + node_index;
            
            mat4 node_matrix;
            glm_mat4_identity(node_matrix);

            int transform_count = 0;

            for (int animation_index = 0; animation_index < model->animation_count; animation_index++)
            {
                if (!BIT_VECTOR_TEST_BIT(object->animations.active_animations_bit_vector, animation_index))
                    continue;
                
                struct model_animation_state *animation_state = object->animations.states + animation_index;
                glm_mat4_mul(animation_state->node_matrices[node_index], node_matrix, node_matrix);
                
                transform_count++;
            }

            if (!transform_count)
            {
                glm_mat4_copy(node->offset_matrix, node_matrix);
            }
            
            shader_set_mat4_v(shader_index, node_matrix, "node_matrices[%i]", node_index);
        }
        
        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;

            struct material_data *material = model->materials + part->material_index;
            render_set_material_uniforms(shader_index, material);

            glDrawElements(GL_TRIANGLES, part->index_count, GL_UNSIGNED_INT, (const void *)(part->index_start * sizeof(int)));

            shader_unbind_textures(shader_index);
        }
    }
}
