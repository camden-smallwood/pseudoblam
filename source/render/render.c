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
#include "render/lights.h"
#include "render/shaders.h"
#include "render/render.h"
#include "textures/dds.h"

/* ---------- private constants */

enum render_flags
{
    _render_input_tab_bit,
    _render_input_h_bit,
};

/* ---------- private variables */

struct render_globals
{
    unsigned int flags;

    int screen_width;
    int screen_height;
    
    int sample_count;

    struct camera_data camera;

    GLuint quad_texture;
    GLuint quad_renderbuffer;
    GLuint quad_framebuffer;
    GLuint quad_intermediate_framebuffer;
    GLuint quad_intermediate_texture;
    GLuint quad_vertex_array;
    GLuint quad_vertex_buffer;
    int quad_shader;

    GLuint shadow_framebuffer;
    GLuint shadow_texture;
    int shadow_width;
    int shadow_height;
    int shadow_shader;

    mat4 light_space_matrix;

    int blinn_phong_shader;

    GLuint default_diffuse_texture;
    GLuint default_specular_texture;
    GLuint default_normals_texture;
    GLuint default_emissive_texture;

    int weapon_model_index;

    int flashlight_light_index;
} static render_globals;

/* ---------- private prototypes */

static void render_initialize_gl(void);
static void render_initialize_quad(void);
static void render_initialize_shadows(void);
static void render_initialize_scene(void);
static void render_initialize_models(void);

static void render_update_input(void);
static void render_update_flashlight(void);

static void render_frame(void);
static void render_quad(void);
static void render_shadows(void);
static void render_models(void);

static void render_model(int shader_index, int model_index, mat4 model_matrix);
static void render_lights(int shader_index);
static void render_material(int shader_index, struct material_data *material);

/* ---------- public code */

void render_initialize(void)
{
    memset(&render_globals, 0, sizeof(render_globals));

    render_globals.screen_width = 1280;
    render_globals.screen_height = 720;

    render_globals.sample_count = 4;

    render_initialize_gl();
    render_initialize_quad();
    render_initialize_shadows();
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

    // Resize the quad texture
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, render_globals.quad_texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, render_globals.sample_count, GL_RGB, render_globals.screen_width, render_globals.screen_height, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Resize the quad intermediate texture
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_intermediate_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_globals.screen_width, render_globals.screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Resize the quad renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, render_globals.quad_renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, render_globals.sample_count, GL_DEPTH24_STENCIL8, render_globals.screen_width, render_globals.screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Resize the viewport
    glViewport(0, 0, width, height);
    
    camera_handle_screen_resize(&render_globals.camera, width, height);
}

void render_update(float delta_ticks)
{
    render_update_input();
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
    
    render_globals.quad_shader = shader_new("../assets/shaders/quad.vs", "../assets/shaders/quad.fs");
    shader_bind_vertex_attributes(render_globals.quad_shader, _vertex_type_flat);

    glGenFramebuffers(1, &render_globals.quad_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_globals.quad_framebuffer);
    
    glGenTextures(1, &render_globals.quad_texture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, render_globals.quad_texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, render_globals.sample_count, GL_RGB, render_globals.screen_width, render_globals.screen_height, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, render_globals.quad_texture, 0);
    
    glGenRenderbuffers(1, &render_globals.quad_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_globals.quad_renderbuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, render_globals.sample_count, GL_DEPTH24_STENCIL8, render_globals.screen_width, render_globals.screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_globals.quad_renderbuffer);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &render_globals.quad_intermediate_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_globals.quad_intermediate_framebuffer);
    
    glGenTextures(1, &render_globals.quad_intermediate_texture);
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_intermediate_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_globals.screen_width, render_globals.screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_globals.quad_intermediate_texture, 0);

    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_initialize_shadows(void)
{
    render_globals.shadow_width = 1024;
    render_globals.shadow_height = 1024;

    render_globals.shadow_shader = shader_new("../assets/shaders/shadows.vs", "../assets/shaders/shadows.fs");

    glGenFramebuffers(1, &render_globals.shadow_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_globals.shadow_framebuffer);

    glGenTextures(1, &render_globals.shadow_texture);
    glBindTexture(GL_TEXTURE_2D, render_globals.shadow_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, render_globals.shadow_width, render_globals.shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (vec4){1, 1, 1, 1});
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, render_globals.shadow_texture, 0);
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_initialize_scene(void)
{
    render_globals.blinn_phong_shader = shader_new("../assets/shaders/generic.vs", "../assets/shaders/blinnphong.fs");

    render_globals.default_diffuse_texture = dds_import_file_as_texture2d("../assets/textures/bricks_diffuse.dds");
    render_globals.default_specular_texture = dds_import_file_as_texture2d("../assets/textures/white.dds");
    render_globals.default_normals_texture = dds_import_file_as_texture2d("../assets/textures/bricks_normal.dds");
    render_globals.default_emissive_texture = dds_import_file_as_texture2d("../assets/textures/black.dds");

    camera_initialize(&render_globals.camera);

    model_import_from_file(_vertex_type_rigid, "../assets/models/crate_space.fbx");
    model_import_from_file(_vertex_type_rigid, "../assets/models/plane.fbx");

    render_globals.weapon_model_index = model_import_from_file(_vertex_type_rigid, "../assets/models/assault_rifle.fbx");
    
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
    glm_vec3_copy((vec3){1.2f, 3.0f, 2.0f}, light->position);
    glm_vec3_copy((vec3){0.8f, 0.2f, 0.1f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){0.7f, 2.0f, 1.0f}, light->position);
    glm_vec3_copy((vec3){0.1f, 0.2f, 0.8f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){2.3f, 3.3f, -4.0f}, light->position);
    glm_vec3_copy((vec3){0.1f, 0.8f, 0.2f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){-4.0f, 2.0f, -12.0f}, light->position);
    glm_vec3_copy((vec3){0.8f, 0.2f, 0.8f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 10.0f;
    light->linear = 0.9f;
    light->quadratic = 0.32f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){0.0f, 4.0f, -3.0}, light->position);
    glm_vec3_copy((vec3){0.8f, 0.8f, 0.8f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;
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
                // TODO: get default texture for specific material model
                puts("using default blinn-phong textures");

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

            shader_bind_vertex_attributes(render_globals.shadow_shader, mesh->vertex_type);
            shader_bind_vertex_attributes(render_globals.blinn_phong_shader, mesh->vertex_type);
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
        
        if (render_globals.camera.movement_speed < 100.0f)
            render_globals.camera.movement_speed *= 2.0f;
        else
            render_globals.camera.movement_speed = 1.0f;
    }
}

static void render_update_flashlight(void)
{
    struct light_data *light = light_get_data(render_globals.flashlight_light_index);

    if (!light)
        return;

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
    render_shadows();
    render_models();
    render_quad();
}

static void render_quad(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // TODO: move to texture abstraction code
    int texture_index = 0;
    glActiveTexture(GL_TEXTURE0 + texture_index);
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_intermediate_texture);

    shader_use(render_globals.quad_shader);
    shader_set_int(render_globals.quad_shader, "quad_texture", texture_index);
    
    glBindVertexArray(render_globals.quad_vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void render_shadows(void)
{
    float near_plane = 1.0f; // TODO
    float far_plane = 7.5f; // TODO

    mat4 lightProjection;
    glm_ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane, lightProjection);
    //glm_perspective(glm_rad(45.0f), (GLfloat)render_globals.shadow_width / (GLfloat)render_globals.shadow_height, near_plane, far_plane, lightProjection);

    struct light_data *flashlight = light_get_data(render_globals.flashlight_light_index);

    mat4 lightView;
    glm_lookat(flashlight->position, flashlight->direction, (vec3){0, 1, 0}, lightView);

    glm_mat4_mul(lightProjection, lightView, render_globals.light_space_matrix);

    shader_use(render_globals.shadow_shader);
    shader_set_mat4(render_globals.shadow_shader, "light_space_matrix", render_globals.light_space_matrix);

    glViewport(0, 0, render_globals.shadow_width, render_globals.shadow_height);
    glBindFramebuffer(GL_FRAMEBUFFER, render_globals.shadow_framebuffer);
    
    glClear(GL_DEPTH_BUFFER_BIT);

    struct model_iterator iterator;
    model_iterator_new(&iterator);

    while (model_iterator_next(&iterator) != -1)
    {
        if (iterator.index == render_globals.weapon_model_index)
            continue;
        
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);

        shader_set_mat4(render_globals.shadow_shader, "model", model_matrix);
        
        for (int mesh_index = 0; mesh_index < iterator.data->mesh_count; mesh_index++)
        {
            struct model_mesh *mesh = iterator.data->meshes + mesh_index;

            glBindVertexArray(mesh->vertex_array);

            for (int part_index = 0; part_index < mesh->part_count; part_index++)
            {
                struct model_mesh_part *part = mesh->parts + part_index;

                glDrawArrays(GL_TRIANGLES, part->vertex_index, part->vertex_count);
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, render_globals.screen_width, render_globals.screen_height);
}

static void render_models(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, render_globals.quad_framebuffer);
    glEnable(GL_DEPTH_TEST);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    struct model_iterator iterator;
    model_iterator_new(&iterator);

    while (model_iterator_next(&iterator) != -1)
    {
        if (iterator.index == render_globals.weapon_model_index)
            continue;
        
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);
        
        render_model(render_globals.blinn_phong_shader, iterator.index, model_matrix);
    }

    if (render_globals.weapon_model_index != -1)
    {
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);
        glm_scale_uni(model_matrix, 0.1f);
        glm_rotate(model_matrix, 1.4f, (vec3){0, 1, 0});
        glm_rotate(model_matrix, -0.05f, (vec3){0, 0, 1});
        glm_translate(model_matrix, (vec3){0.6f, -0.725f, 0.3f});

        mat4 inverted_view;
        glm_mat4_inv(render_globals.camera.view, inverted_view);
        glm_mat4_mul(inverted_view, model_matrix, model_matrix);

        render_model(render_globals.blinn_phong_shader, render_globals.weapon_model_index, model_matrix);
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, render_globals.quad_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_globals.quad_intermediate_framebuffer);

    glBlitFramebuffer(
        0, 0, render_globals.screen_width, render_globals.screen_height,
        0, 0, render_globals.screen_width, render_globals.screen_height,
        GL_COLOR_BUFFER_BIT,
        GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_model(int shader_index, int model_index, mat4 model_matrix)
{
    struct model_data *model = model_get_data(model_index);

    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        shader_use(shader_index);

        shader_set_vec3(shader_index, "camera_position", render_globals.camera.position);

        shader_set_mat4(shader_index, "light_space_matrix", render_globals.light_space_matrix);
        shader_set_mat4(shader_index, "model", model_matrix);
        shader_set_mat4(shader_index, "view", render_globals.camera.view);
        shader_set_mat4(shader_index, "projection", render_globals.camera.projection);

        shader_set_bool(shader_index, "use_nodes", mesh->vertex_type == _vertex_type_skinned);
        shader_set_int(shader_index, "node_count", model->node_count);

        for (int node_index = 0; node_index < model->node_count; node_index++)
        {
            struct model_node *node = model->nodes + node_index;
            // TODO: get node matrix from animation state
            
            shader_set_mat4_v(shader_index, node->offset_matrix, "node_matrices[%i]", node_index);
        }
        
        render_lights(shader_index);

        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;

            struct material_data *material = model->materials + part->material_index;
            render_material(shader_index, material);

            glActiveTexture(GL_TEXTURE31);
            glBindTexture(GL_TEXTURE_2D, render_globals.shadow_texture);

            shader_set_int(render_globals.blinn_phong_shader, "shadow_texture", render_globals.shadow_texture);

            glDrawArrays(GL_TRIANGLES, part->vertex_index, part->vertex_count);

            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                glActiveTexture(GL_TEXTURE0 + texture_index);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        shader_set_bool(shader_index, "use_nodes", false);
        shader_set_int(shader_index, "node_count", 0);
    }
}

static void render_lights(int shader_index)
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

    shader_set_uint(shader_index, "directional_light_count", light_counts[_light_type_directional]);
    shader_set_uint(shader_index, "point_light_count", light_counts[_light_type_point]);
    shader_set_uint(shader_index, "spot_light_count", light_counts[_light_type_spot]);
}

static void render_material(int shader_index, struct material_data *material)
{
    shader_set_vec3(shader_index, "material.diffuse_color", material->base_properties.color_diffuse);

    shader_set_vec3(shader_index, "material.specular_color", material->base_properties.color_specular);
    shader_set_float(shader_index, "material.specular_amount", material->specular_properties.specular_factor);
    shader_set_float(shader_index, "material.specular_shininess", material->specular_properties.glossiness_factor);

    shader_set_vec3(shader_index, "material.ambient_color", material->base_properties.color_ambient);
    shader_set_float(shader_index, "material.ambient_amount", 0.1f);

    shader_set_float(shader_index, "material.bump_scaling", material->base_properties.bump_scaling);

    for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
    {
        struct material_texture *texture = material->textures + texture_index;

        glActiveTexture(GL_TEXTURE0 + texture_index);
        glBindTexture(GL_TEXTURE_2D, texture->id);

        switch (texture->usage)
        {
        case _material_texture_usage_diffuse:
            shader_set_int(shader_index, "material.diffuse_texture", texture_index);
            break;

        case _material_texture_usage_specular:
            shader_set_int(shader_index, "material.specular_texture", texture_index);
            break;

        case _material_texture_usage_emissive:
            shader_set_int(shader_index, "material.emissive_texture", texture_index);
            break;

        case _material_texture_usage_normals:
            shader_set_int(shader_index, "material.normal_texture", texture_index);
            break;

        default:
            fprintf(stderr, "ERROR: unhandled texture usage - \"%s\"\n", material_texture_usage_to_string(texture->usage));
            exit(EXIT_FAILURE);
        }
    }
}
