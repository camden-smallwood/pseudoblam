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

    GLuint default_diffuse_texture;
    GLuint default_specular_texture;
    GLuint default_normals_texture;
    GLuint default_emissive_texture;

    int cube_model_index;
    struct model_animation_manager cube_animations;

    int weapon_model_index;
    int flashlight_light_index;
} static render_globals;

/* ---------- private prototypes */

static void render_initialize_gl(void);

static void render_initialize_geometry_pass(void);
static void render_initialize_depth_pass(void);
static void render_initialize_occlusion_pass(void);
static void render_initialize_lighting_pass(void);
static void render_initialize_transparent_pass(void);
static void render_initialize_postprocess_pass(void);
static void render_initialize_postprocess_blur_pass(void);
static void render_initialize_postprocess_hdr_pass(void);
static void render_initialize_quad(void);

static void render_initialize_scene(void);
static void render_initialize_models(void);

static void render_update_input(void);
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

static void render_model(int shader_index, int model_index, mat4 model_matrix);

/* ---------- public code */

void render_initialize(void)
{
    memset(&render_globals, 0, sizeof(render_globals));

    render_globals.screen_width = 1280;
    render_globals.screen_height = 720;

    render_globals.sample_count = 4;

    render_initialize_gl();
    
    render_initialize_geometry_pass();
    render_initialize_depth_pass();
    render_initialize_occlusion_pass();
    render_initialize_lighting_pass();
    render_initialize_transparent_pass();
    render_initialize_postprocess_pass();
    render_initialize_quad();
    
    render_initialize_scene();
    render_initialize_models();
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
    render_update_input();
    camera_update(&render_globals.camera, delta_ticks);
    render_update_flashlight();
    
    model_animations_update(&render_globals.cube_animations, delta_ticks);

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

static void render_initialize_scene(void)
{
    render_globals.default_diffuse_texture = dds_import_file_as_texture2d("../assets/textures/bricks_diffuse.dds");
    render_globals.default_specular_texture = dds_import_file_as_texture2d("../assets/textures/white.dds");
    render_globals.default_normals_texture = dds_import_file_as_texture2d("../assets/textures/bricks_normal.dds");
    render_globals.default_emissive_texture = dds_import_file_as_texture2d("../assets/textures/black.dds");

    camera_initialize(&render_globals.camera);

    model_import_from_file(_vertex_type_rigid, "../assets/models/plane.fbx");

    render_globals.weapon_model_index = -1;//model_import_from_file(_vertex_type_skinned, "../assets/models/assault_rifle.fbx");
    
    render_globals.cube_model_index = model_import_from_file(_vertex_type_skinned, "../assets/models/grunt.fbx");
    model_animations_initialize(&render_globals.cube_animations, render_globals.cube_model_index);
    model_set_animation_active(&render_globals.cube_animations, 0, true);

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
    struct model_iterator iterator;
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
                        .id = render_globals.default_diffuse_texture,
                    },
                    {
                        .usage = _material_texture_usage_specular,
                        .id = render_globals.default_specular_texture,
                    },
                    {
                        .usage = _material_texture_usage_normals,
                        .id = render_globals.default_normals_texture,
                    },
                    {
                        .usage = _material_texture_usage_emissive,
                        .id = render_globals.default_emissive_texture,
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

                if (texture->id)
                    continue;
                
                switch (texture->usage)
                {
                case _material_texture_usage_diffuse:
                    texture->id = render_globals.default_diffuse_texture;
                    break;

                case _material_texture_usage_specular:
                    texture->id = render_globals.default_specular_texture;
                    break;

                case _material_texture_usage_emissive:
                    texture->id = render_globals.default_emissive_texture;
                    break;

                case _material_texture_usage_normals:
                    texture->id = render_globals.default_normals_texture;
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

            shader_bind_vertex_attributes(render_globals.geometry_pass.shader_index, mesh->vertex_type);
        }
    }
}

static void render_update_input(void)
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
}

static void render_update_flashlight(void)
{
    struct light_data *light = light_get_data(render_globals.flashlight_light_index);
    
    if (!light)
    {
        return;
    }

    glm_vec3_copy(render_globals.camera.position, light->position);

    vec3 offset;
    glm_vec3_zero(offset);
    glm_vec3_scale(render_globals.camera.forward, 0.1f, offset);
    glm_vec3_add(light->position, offset, light->position);

    glm_vec3_copy(render_globals.camera.forward, light->direction);
    glm_vec3_rotate(light->direction, glm_rad(1.4f), render_globals.camera.up);
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

    struct model_iterator iterator;
    model_iterator_new(&iterator);

    while (model_iterator_next(&iterator) != -1)
    {
        if (iterator.index == render_globals.weapon_model_index)
            continue;
        
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);
        
        render_model(render_globals.geometry_pass.shader_index, iterator.index, model_matrix);
    }

    if (render_globals.weapon_model_index != -1)
    {
        int model_index = render_globals.weapon_model_index;

        mat4 model_matrix;
        glm_mat4_identity(model_matrix);

        mat4 model_scale_matrix;
        glm_mat4_identity(model_scale_matrix);
        glm_scale_uni(model_scale_matrix, 0.1f);

        mat4 model_rotation_matrix;
        glm_mat4_identity(model_rotation_matrix);
        glm_rotate(model_rotation_matrix, glm_rad(-90.0), (vec3){1, 0, 0});
        glm_rotate(model_rotation_matrix, glm_rad(0.0), (vec3){0, 1, 0});
        glm_rotate(model_rotation_matrix, glm_rad(0.0), (vec3){0, 0, 1});

        mat4 model_position_matrix;
        glm_mat4_identity(model_position_matrix);
        glm_translate(model_position_matrix, (vec3){4, 50, -7});

        glm_mat4_mul(model_matrix, model_scale_matrix, model_matrix);
        glm_mat4_mul(model_matrix, model_rotation_matrix, model_matrix);
        glm_mat4_mul(model_matrix, model_position_matrix, model_matrix);

        mat4 inverted_view;
        glm_mat4_inv(render_globals.camera.view, inverted_view);
        glm_mat4_mul(inverted_view, model_matrix, model_matrix);

        render_model(render_globals.geometry_pass.shader_index, model_index, model_matrix);
    }

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

    struct texture_data *noise_texture = texture_get_data(render_globals.occlusion_pass.noise_texture_index);
    shader_bind_texture(render_globals.occlusion_pass.shader_index, noise_texture->id, "noise_texture");
    
    struct texture_data *view_normal_texture = texture_get_data(render_globals.geometry_pass.view_normal_texture_index);
    shader_bind_texture(render_globals.occlusion_pass.shader_index, view_normal_texture->id, "normal_texture");
    
    struct texture_data *depth_texture = texture_get_data(render_globals.depth_pass.texture_index);
    shader_bind_texture(render_globals.occlusion_pass.shader_index, depth_texture->id, "depth_texture");
    
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

    struct texture_data *position_texture = texture_get_data(render_globals.geometry_pass.position_texture_index);
    shader_bind_texture(render_globals.lighting_pass.shader_index, position_texture->id, "position_texture");

    struct texture_data *normal_texture = texture_get_data(render_globals.geometry_pass.normal_texture_index);
    shader_bind_texture(render_globals.lighting_pass.shader_index, normal_texture->id, "normal_texture");

    struct texture_data *albedo_specular_texture = texture_get_data(render_globals.geometry_pass.albedo_specular_texture_index);
    shader_bind_texture(render_globals.lighting_pass.shader_index, albedo_specular_texture->id, "albedo_specular_texture");

    struct texture_data *material_texture = texture_get_data(render_globals.geometry_pass.material_texture_index);
    shader_bind_texture(render_globals.lighting_pass.shader_index, material_texture->id, "material_texture");

    struct texture_data *emissive_texture = texture_get_data(render_globals.geometry_pass.emissive_texture_index);
    shader_bind_texture(render_globals.lighting_pass.shader_index, emissive_texture->id, "emissive_texture");
    
    struct texture_data *ssao_texture = texture_get_data(render_globals.occlusion_pass.base_texture_index);
    shader_bind_texture(render_globals.lighting_pass.shader_index, ssao_texture->id, "ssao_texture");
    
    render_set_lighting_uniforms(render_globals.lighting_pass.shader_index);

    // TODO: draw as mesh?
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
        
        glClear(GL_COLOR_BUFFER_BIT);

        shader_use(render_globals.postprocess_pass.blur_pass.shader_index);

        struct texture_data *blur_texture = texture_get_data(render_globals.postprocess_pass.texture_index);
        shader_bind_texture(render_globals.postprocess_pass.blur_pass.shader_index, blur_texture->id, "blur_texture");
        
        shader_set_bool(render_globals.postprocess_pass.blur_pass.shader_index, blur_horizontal, "blur_horizontal");
        
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
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);

    framebuffer_use(&render_globals.postprocess_pass.hdr_pass.framebuffer);
    
    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(render_globals.postprocess_pass.hdr_pass.shader_index);
    
    struct texture_data *base_texture = texture_get_data(render_globals.lighting_pass.base_texture_index);
    shader_bind_texture(render_globals.postprocess_pass.hdr_pass.shader_index, base_texture->id, "base_texture");
    
    struct texture_data *hdr_texture = texture_get_data(render_globals.postprocess_pass.texture_index);
    shader_bind_texture(render_globals.postprocess_pass.hdr_pass.shader_index, hdr_texture->id, "hdr_texture");
    
    shader_set_bool(render_globals.postprocess_pass.hdr_pass.shader_index, true, "bloom");
    
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.postprocess_pass.hdr_pass.shader_index);

    framebuffer_use(NULL);
}

static void render_quad(void)
{
    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
    glDisable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT);

    shader_use(render_globals.quad_shader);
    
    struct texture_data *quad_texture = texture_get_data(render_globals.postprocess_pass.hdr_pass.texture_index);
    shader_bind_texture(render_globals.quad_shader, quad_texture->id, "quad_texture");

    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader_unbind_textures(render_globals.quad_shader);
}

static void render_set_lighting_uniforms(int shader_index)
{
    struct light_iterator light_iterator;
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

        shader_bind_texture(shader_index, texture->id, texture_variable_name);
    }
}

static void render_model(int shader_index, int model_index, mat4 model_matrix)
{
    struct model_data *model = model_get_data(model_index);

    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        shader_use(shader_index);

        shader_set_mat4(shader_index, model_matrix, "model");
        shader_set_mat4(shader_index, render_globals.camera.view, "view");
        shader_set_mat4(shader_index, render_globals.camera.projection, "projection");

        shader_set_bool(shader_index, mesh->vertex_type == _vertex_type_skinned, "use_nodes");
        shader_set_int(shader_index, model->node_count, "node_count");

        for (int node_index = 0; node_index < model->node_count; node_index++)
        {
            struct model_node *node = model->nodes + node_index;
            
            mat4 node_matrix;

            if (model_index == render_globals.cube_model_index)
                glm_mat4_copy(render_globals.cube_animations.animation_states[0].node_matrices[node_index], node_matrix);
            else
                glm_mat4_copy(node->offset_matrix, node_matrix);
            
            shader_set_mat4_v(shader_index, node_matrix, "node_matrices[%i]", node_index);
        }
        
        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;

            struct material_data *material = model->materials + part->material_index;
            render_set_material_uniforms(shader_index, material);

            glDrawElements(GL_TRIANGLES, part->index_count, GL_UNSIGNED_INT, (const void *)(part->index_start * sizeof(int)));

            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                shader_unbind_texture(shader_index, texture_index);
            }
        }

        shader_set_bool(shader_index, false, "use_nodes");
        shader_set_int(shader_index, 0, "node_count");
    }
}
