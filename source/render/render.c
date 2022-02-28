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

    struct camera_data camera;

    GLuint quad_texture;
    GLuint quad_renderbuffer;
    GLuint quad_framebuffer;
    GLuint quad_vertex_array;
    GLuint quad_vertex_buffer;
    int quad_shader;

    int blinn_phong_shader;

    GLuint default_diffuse_texture;
    GLuint default_specular_texture;
    GLuint default_normal_texture;
    GLuint default_emissive_texture;

    int weapon_model_index;

    int headlight_light_index;
} static render_globals;

/* ---------- private prototypes */

static void render_initialize_gl(void);
static void render_initialize_quad(void);

static void render_update_input(void);
static void render_update_headlight(void);

static void render_frame(void);
static void render_quad(void);
static void render_models(void);
static void render_model(struct model_data *model, mat4 model_matrix);

/* ---------- public code */

void render_initialize(void)
{
    memset(&render_globals, 0, sizeof(render_globals));

    render_globals.screen_width = 1280;
    render_globals.screen_height = 720;

    render_initialize_gl();
    render_initialize_quad();
    
    camera_initialize(&render_globals.camera);

    render_globals.blinn_phong_shader = shader_new("../assets/shaders/generic.vs", "../assets/shaders/blinnphong.fs");

    render_globals.default_diffuse_texture = dds_import_file_as_texture2d("../assets/textures/bricks_diffuse.dds");
    render_globals.default_specular_texture = dds_import_file_as_texture2d("../assets/textures/white.dds");
    render_globals.default_normal_texture = dds_import_file_as_texture2d("../assets/textures/bricks_normal.dds");
    render_globals.default_emissive_texture = dds_import_file_as_texture2d("../assets/textures/black.dds");

    model_import_from_file(_vertex_type_rigid, "../assets/models/plane.fbx");
    model_import_from_file(_vertex_type_rigid, "../assets/models/cube.fbx");
    
    render_globals.weapon_model_index = model_import_from_file(_vertex_type_skinned, "../assets/models/assault_rifle_skinned.fbx");
    
    struct model_iterator iterator;
    model_iterator_new(&iterator);

    while (model_iterator_next(&iterator) != -1)
    {
        for (int mesh_index = 0; mesh_index < iterator.data->mesh_count; mesh_index++)
        {
            struct model_mesh *mesh = iterator.data->meshes + mesh_index;

            if (!mesh->vertex_data)
                continue;
            
            const struct vertex_definition *vertex_definition = vertex_definition_get(mesh->vertex_type);

            // Create and bind the mesh's vertex array
            glGenVertexArrays(1, &mesh->vertex_array);
            glBindVertexArray(mesh->vertex_array);

            // Create, bind and fill the mesh's vertex buffer
            glGenBuffers(1, &mesh->vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * vertex_definition->size, mesh->vertex_data, GL_STATIC_DRAW);

            shader_bind_vertex_attributes(render_globals.blinn_phong_shader, mesh->vertex_type);
        }
    }

    // Initialize lights
    {
        struct light_data *light;

        render_globals.headlight_light_index = light_new();
        light = light_get_data(render_globals.headlight_light_index);
        light->type = _light_type_spot;
        SET_BIT(light->flags, _light_is_hidden_bit, true);
        glm_vec3_copy(render_globals.camera.position, light->position);
        glm_vec3_copy(render_globals.camera.forward, light->direction);
        glm_vec3_copy((vec3){1, 1, 1}, light->diffuse_color);
        glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
        glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
        light->constant = 1.0f;
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
        glm_vec3_copy((vec3){0.7f, 0.2f, 2.0f}, light->position);
        glm_vec3_copy((vec3){0.1f, 0.2f, 0.8f}, light->diffuse_color);
        glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
        glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
        light->constant = 1.0f;
        light->linear = 0.09f;
        light->quadratic = 0.032f;

        light = light_get_data(light_new());
        light->type = _light_type_point;
        glm_vec3_copy((vec3){2.3f, -3.3f, -4.0f}, light->position);
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
        glm_vec3_copy((vec3){0.0f, 0.0f, -3.0}, light->position);
        glm_vec3_copy((vec3){0.8f, 0.8f, 0.8f}, light->diffuse_color);
        glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
        glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
        light->constant = 1.0f;
        light->linear = 0.09f;
        light->quadratic = 0.032f;
    }
}

void render_dispose(void)
{
    // TODO: finish
}

void render_handle_screen_resize(int width, int height)
{
    render_globals.screen_width = width;
    render_globals.screen_height = height;

    // Resize the viewport
    glViewport(0, 0, width, height);

    // Resize the quad texture
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Resize the quad renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, render_globals.quad_renderbuffer); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    camera_handle_screen_resize(&render_globals.camera, width, height);
}

void render_update(float delta_ticks)
{
    render_update_input();
    camera_update(&render_globals.camera, delta_ticks);
    render_update_headlight();
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

    // Enable multisampling
    glEnable(GL_MULTISAMPLE);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Enable backface culling
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
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_globals.screen_width, render_globals.screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_globals.quad_texture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &render_globals.quad_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_globals.quad_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, render_globals.screen_width, render_globals.screen_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_globals.quad_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_update_input(void)
{
    // Toggle the headlight on or off
    if (input_is_key_down(SDL_SCANCODE_H))
    {
        SET_BIT(render_globals.flags, _render_input_h_bit, true);
    }
    else if (TEST_BIT(render_globals.flags, _render_input_h_bit))
    {
        SET_BIT(render_globals.flags, _render_input_h_bit, false);

        light_set_hidden(
            render_globals.headlight_light_index,
            !light_is_hidden(render_globals.headlight_light_index));
    }

    // Cycle through camera speed intervals when the tab key is pressed and then released
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

static void render_update_headlight(void)
{
    struct light_data *headlight = light_get_data(render_globals.headlight_light_index);

    if (!headlight)
        return;

    glm_vec3_copy(render_globals.camera.position, headlight->position);
    glm_vec3_copy(render_globals.camera.forward, headlight->direction);
}

static void render_frame(void)
{
    render_models();
    render_quad();
}

static void render_quad(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // TODO: move to texture abstraction code
    int texture_index = 0;
    glActiveTexture(GL_TEXTURE0 + texture_index);
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_texture);

    shader_use(render_globals.quad_shader);
    shader_set_int(render_globals.quad_shader, "quad_texture", texture_index);

    glBindVertexArray(render_globals.quad_vertex_array);
    glBindTexture(GL_TEXTURE_2D, render_globals.quad_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
        
        render_model(iterator.data, model_matrix);
    }

    if (render_globals.weapon_model_index != -1)
    {
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);
        glm_scale_uni(model_matrix, 0.2f);
        glm_rotate(model_matrix, 1.4f, (vec3){0, 1, 0});
        glm_rotate(model_matrix, -0.05f, (vec3){0, 0, 1});
        glm_translate(model_matrix, (vec3){0.6f, -0.725f, 0.3f});

        mat4 inverted_view;
        glm_mat4_inv(render_globals.camera.view, inverted_view);
        glm_mat4_mul(inverted_view, model_matrix, model_matrix);

        render_model(model_get_data(render_globals.weapon_model_index), model_matrix);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
}

static void render_model(struct model_data *model, mat4 model_matrix)
{
    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        struct shader_data *blinn_phong_shader = shader_get_data(render_globals.blinn_phong_shader);

        // Bind the shader
        glUseProgram(blinn_phong_shader->program);

        // Bind the camera position
        shader_set_vec3(render_globals.blinn_phong_shader, "camera_position", render_globals.camera.position);

        // Bind the camera model/view/projection matrices
        shader_set_mat4(render_globals.blinn_phong_shader, "model", model_matrix);
        shader_set_mat4(render_globals.blinn_phong_shader, "view", render_globals.camera.view);
        shader_set_mat4(render_globals.blinn_phong_shader, "projection", render_globals.camera.projection);

        // Bind the lighting uniforms
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

            char uniform_name[256];

            shader_set_vec3_v(render_globals.blinn_phong_shader, light->position, "%s[%i].position", lights_array_name, light_counts[light->type]);
            shader_set_vec3_v(render_globals.blinn_phong_shader, light->diffuse_color, "%s[%i].diffuse_color", lights_array_name, light_counts[light->type]);
            shader_set_vec3_v(render_globals.blinn_phong_shader, light->ambient_color, "%s[%i].ambient_color", lights_array_name, light_counts[light->type]);
            shader_set_vec3_v(render_globals.blinn_phong_shader, light->specular_color, "%s[%i].specular_color", lights_array_name, light_counts[light->type]);

            if (light->type == _light_type_directional || light->type == _light_type_spot)
            {
                shader_set_vec3_v(render_globals.blinn_phong_shader, light->direction, "%s[%i].direction", lights_array_name, light_counts[light->type]);
            }

            if (light->type == _light_type_point || light->type == _light_type_spot)
            {
                shader_set_float_v(render_globals.blinn_phong_shader, light->constant, "%s[%i].constant", lights_array_name, light_counts[light->type]);
                shader_set_float_v(render_globals.blinn_phong_shader, light->linear, "%s[%i].linear", lights_array_name, light_counts[light->type]);
                shader_set_float_v(render_globals.blinn_phong_shader, light->quadratic, "%s[%i].quadratic", lights_array_name, light_counts[light->type]);
            }

            if (light->type == _light_type_spot)
            {
                shader_set_float_v(render_globals.blinn_phong_shader, cosf(glm_rad(light->inner_cutoff)), "%s[%i].inner_cutoff", lights_array_name, light_counts[light->type]);
                shader_set_float_v(render_globals.blinn_phong_shader, cosf(glm_rad(light->outer_cutoff)), "%s[%i].outer_cutoff", lights_array_name, light_counts[light->type]);
            }

            light_counts[light->type]++;
        }

        // Bind the total number of lights for each light type
        shader_set_uint(render_globals.blinn_phong_shader, "directional_light_count", light_counts[_light_type_directional]);
        shader_set_uint(render_globals.blinn_phong_shader, "point_light_count", light_counts[_light_type_point]);
        shader_set_uint(render_globals.blinn_phong_shader, "spot_light_count", light_counts[_light_type_spot]);

        // Draw the geometry
        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;
            struct material_data *material = model->materials + part->material_index;

            // Bind the material uniforms
            shader_set_vec3(render_globals.blinn_phong_shader, "material.diffuse_color", material->base_properties.color_diffuse);

            shader_set_vec3(render_globals.blinn_phong_shader, "material.specular_color", material->base_properties.color_specular);
            shader_set_float(render_globals.blinn_phong_shader, "material.specular_amount", material->specular_properties.specular_factor);
            shader_set_float(render_globals.blinn_phong_shader, "material.specular_shininess", material->specular_properties.glossiness_factor);

            shader_set_vec3(render_globals.blinn_phong_shader, "material.ambient_color", material->base_properties.color_ambient);
            shader_set_float(render_globals.blinn_phong_shader, "material.ambient_amount", 0.1f);

            shader_set_float(render_globals.blinn_phong_shader, "material.bump_scaling", material->base_properties.bump_scaling);

            // TODO: determine required default textures ahead of time

            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                struct material_texture *texture = material->textures + texture_index;

                glActiveTexture(GL_TEXTURE0 + texture_index);
                glBindTexture(GL_TEXTURE_2D, texture->id);

                switch (texture->usage)
                {
                case _material_texture_usage_diffuse:
                    shader_set_int(render_globals.blinn_phong_shader, "material.diffuse_texture", texture_index);
                    break;

                case _material_texture_usage_specular:
                    shader_set_int(render_globals.blinn_phong_shader, "material.specular_texture", texture_index);
                    break;

                case _material_texture_usage_emissive:
                    shader_set_int(render_globals.blinn_phong_shader, "material.emissive_texture", texture_index);
                    break;

                case _material_texture_usage_normals:
                    shader_set_int(render_globals.blinn_phong_shader, "material.normal_texture", texture_index);
                    break;

                default:
                    fprintf(stderr, "ERROR: unhandled texture usage - \"%s\"\n", material_texture_usage_to_string(texture->usage));
                    exit(EXIT_FAILURE);
                }
            }

            glDrawArrays(GL_TRIANGLES, part->vertex_index, part->vertex_count);

            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                glActiveTexture(GL_TEXTURE0 + texture_index);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
    }
}
