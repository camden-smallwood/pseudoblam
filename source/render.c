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

#include "common.h"
#include "camera.h"
#include "dds.h"
#include "input.h"
#include "lights.h"
#include "models.h"
#include "render.h"
#include "shaders.h"

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

    int blinn_phong_shader;

    GLuint diffuse_texture;
    GLuint specular_texture;
    GLuint normal_texture;

    int weapon_model_index;

    int headlight_light_index;
} static render_globals;

/* ---------- private prototypes */

static void render_model(struct model_data *model, mat4 model_matrix);

/* ---------- public code */

void render_initialize(void)
{
    memset(&render_globals, 0, sizeof(render_globals));

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

    camera_initialize(&render_globals.camera);

    render_globals.blinn_phong_shader = shader_new("../assets/shaders/generic.vs", "../assets/shaders/blinnphong.fs");

    render_globals.diffuse_texture = dds_import_file_as_texture2d("../assets/textures/bricks_diffuse.dds");
    render_globals.specular_texture = dds_import_file_as_texture2d("../assets/textures/white.dds");
    render_globals.normal_texture = dds_import_file_as_texture2d("../assets/textures/bricks_normal.dds");

    render_globals.weapon_model_index = model_import_from_file(_vertex_type_rigid, "../assets/models/assault_rifle.dae");
    
    model_import_from_file(_vertex_type_rigid, "../assets/models/plane.obj");
    model_import_from_file(_vertex_type_rigid, "../assets/models/cube.fbx");
    
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
    light->inner_cutoff = 12.5f;
    light->outer_cutoff = 15.0f;
    
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
    glm_vec3_copy((vec3){0.1f, 0.8f, 0.2f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){2.3f, -3.3f, -4.0f}, light->position);
    glm_vec3_copy((vec3){0.1f, 0.2f, 0.8f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;

    light = light_get_data(light_new());
    light->type = _light_type_point;
    glm_vec3_copy((vec3){-4.0f, 2.0f, -12.0f}, light->position);
    glm_vec3_copy((vec3){0.8f, 0.8f, 0.8f}, light->diffuse_color);
    glm_vec3_copy((vec3){0.05f, 0.05f, 0.05f}, light->ambient_color);
    glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, light->specular_color);
    light->constant = 1.0f;
    light->linear = 0.09f;
    light->quadratic = 0.032f;

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

void render_dispose(void)
{
    // TODO: finish
}

void render_handle_screen_resize(int width, int height)
{
    render_globals.screen_width = width;
    render_globals.screen_height = height;

    camera_handle_screen_resize(&render_globals.camera, width, height);
}

void render_update(float delta_ticks)
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
    
    camera_update(&render_globals.camera, delta_ticks);

    // Update the headlight position
    struct light_data *headlight = light_get_data(render_globals.headlight_light_index);
    glm_vec3_copy(render_globals.camera.position, headlight->position);
    glm_vec3_copy(render_globals.camera.forward, headlight->direction);

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
}

/* ---------- private code */

static void render_model(struct model_data *model, mat4 model_matrix)
{
    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        struct shader_data *blinn_phong_shader = shader_get_data(render_globals.blinn_phong_shader);

        // Bind the shader
        glUseProgram(blinn_phong_shader->program);

        // Bind the camera position
        glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, "camera_position"), 1, (const GLfloat *)render_globals.camera.position);

        // Bind the camera model/view/projection matrices
        glUniformMatrix4fv(glGetUniformLocation(blinn_phong_shader->program, "model"), 1, GL_FALSE, (const GLfloat *)model_matrix);
        glUniformMatrix4fv(glGetUniformLocation(blinn_phong_shader->program, "view"), 1, GL_FALSE, (const GLfloat *)render_globals.camera.view);
        glUniformMatrix4fv(glGetUniformLocation(blinn_phong_shader->program, "projection"), 1, GL_FALSE, (const GLfloat *)render_globals.camera.projection);

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

            glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].position", lights_array_name, light_counts[light->type]), uniform_name)), 1, light->position);
            glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].diffuse_color", lights_array_name, light_counts[light->type]), uniform_name)), 1, light->diffuse_color);
            glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].ambient_color", lights_array_name, light_counts[light->type]), uniform_name)), 1, light->ambient_color);
            glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].specular_color", lights_array_name, light_counts[light->type]), uniform_name)), 1, light->specular_color);

            switch (light->type)
            {
            case _light_type_directional:
                glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].direction", lights_array_name, light_counts[light->type]), uniform_name)), 1, light->direction);
                break;
            
            case _light_type_spot:
            case _light_type_point:
                glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].constant", lights_array_name, light_counts[light->type]), uniform_name)), 1, (const GLfloat[]){1.0f});
                glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].linear", lights_array_name, light_counts[light->type]), uniform_name)), 1, (const GLfloat[]){0.09f});
                glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].quadratic", lights_array_name, light_counts[light->type]), uniform_name)), 1, (const GLfloat[]){0.032f});
                if (light->type == _light_type_spot)
                {
                    glUniform3fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].direction", lights_array_name, light_counts[light->type]), uniform_name)), 1, light->direction);
                    glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].inner_cutoff", lights_array_name, light_counts[light->type]), uniform_name)), 1, (const GLfloat[]){cosf(glm_rad(light->inner_cutoff))});
                    glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, (snprintf(uniform_name, sizeof(uniform_name), "%s[%i].outer_cutoff", lights_array_name, light_counts[light->type]), uniform_name)), 1, (const GLfloat[]){cosf(glm_rad(light->outer_cutoff))});
                }
                break;
            
            default:
                fprintf(stderr, "ERROR: unhandled light type %i\n", light->type);
                exit(EXIT_FAILURE);
            }

            light_counts[light->type]++;
        }

        // Bind the total number of lights for each light type
        glUniform1uiv(glGetUniformLocation(blinn_phong_shader->program, "directional_light_count"), 1, (const GLuint[]){light_counts[_light_type_directional]});
        glUniform1uiv(glGetUniformLocation(blinn_phong_shader->program, "point_light_count"), 1, (const GLuint[]){light_counts[_light_type_point]});
        glUniform1uiv(glGetUniformLocation(blinn_phong_shader->program, "spot_light_count"), 1, (const GLuint[]){light_counts[_light_type_spot]});

        // Draw the geometry
        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;
            struct material_data *material = model->materials + part->material_index;

            // Bind the material uniforms
            glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, "material.diffuse_color"), 1, material->base_properties.color_diffuse);

            glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, "material.specular_color"), 1, material->base_properties.color_specular);
            glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, "material.specular_amount"), 1, (const GLfloat[]){material->specular_properties.specular_factor});
            glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, "material.specular_shininess"), 1, (const GLfloat[]){material->specular_properties.glossiness_factor});

            glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, "material.ambient_color"), 1, material->base_properties.color_ambient);
            glUniform1fv(glGetUniformLocation(blinn_phong_shader->program, "material.ambient_amount"), 1, (const GLfloat[]){0.1f});

            // Activate and bind the material's diffuse texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, render_globals.diffuse_texture);
            glUniform1i(glGetUniformLocation(blinn_phong_shader->program, "material.diffuse_texture"), 0);

            // Activate and bind the material's specular texture
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, render_globals.specular_texture);
            glUniform1i(glGetUniformLocation(blinn_phong_shader->program, "material.specular_texture"), 1);

            // Activate and bind the material's normal texture
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, render_globals.normal_texture);
            glUniform1i(glGetUniformLocation(blinn_phong_shader->program, "material.normal_texture"), 2);
            
            for (int texture_index = 0; texture_index < material->texture_count; texture_index++)
            {
                struct material_texture *texture = material->textures + texture_index;

                switch (texture->usage)
                {
                case _material_texture_usage_diffuse:
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture->id);
                    glUniform1i(glGetUniformLocation(blinn_phong_shader->program, "material.diffuse_texture"), 0);
                    break;

                case _material_texture_usage_specular:
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, texture->id);
                    glUniform1i(glGetUniformLocation(blinn_phong_shader->program, "material.specular_texture"), 1);
                    break;

                case _material_texture_usage_normals:
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, texture->id);
                    glUniform1i(glGetUniformLocation(blinn_phong_shader->program, "material.normal_texture"), 2);
                    break;

                default:
                    fprintf(stderr, "ERROR: unhandled texture usage - \"%s\"\n", material_texture_usage_to_string(texture->usage));
                    exit(EXIT_FAILURE);
                }
            }

            glDrawArrays(GL_TRIANGLES, part->vertex_index, part->vertex_count);
        }
    }
}
