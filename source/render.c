#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <SDL_scancode.h>
#include <GL/glew.h>
#include <cglm/cglm.h>

#include "common.h"
#include "input.h"
#include "dds.h"
#include "models.h"
#include "camera.h"
#include "render.h"

/* ---------- private variables */

struct
{
    bool tab_pressed;

    int screen_width;
    int screen_height;

    struct camera_data camera;

    int model_count;
    struct model_data *models;

    GLuint hdr_buffer;
    GLuint render_buffer;

    GLuint program;
    GLuint diffuse_texture;
    GLuint specular_texture;
    GLuint normal_texture;

    int weapon_model_index;
} render_globals;

/* ---------- private prototypes */

static void render_model(int model_index, mat4 model_matrix);

static GLuint render_compile_shader_source(GLenum shader_type, const char *shader_source);
static GLuint render_load_and_compile_shader_file(GLenum shader_type, const char *file_path);
static GLuint render_load_shader_program(const char *vertex_shader_path, const char *fragment_shader_path);

static GLuint render_load_dds_file_as_texture2d(const char *file_path);

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

    render_globals.program = render_load_shader_program("../assets/shaders/generic.vs", "../assets/shaders/blinnphong.fs");
    render_globals.diffuse_texture = render_load_dds_file_as_texture2d("../assets/textures/bricks_diffuse.dds");
    render_globals.specular_texture = render_load_dds_file_as_texture2d("../assets/textures/white.dds");
    render_globals.normal_texture = render_load_dds_file_as_texture2d("../assets/textures/bricks_normal.dds");

    struct model_data model;

    render_globals.weapon_model_index = render_globals.model_count;

    model_import_from_file(&model, "../assets/models/assault_rifle.dae");
    mempush(&render_globals.model_count, (void **)&render_globals.models, &model, sizeof(model), realloc);

    model_import_from_file(&model, "../assets/models/cube_sphere.obj");
    mempush(&render_globals.model_count, (void **)&render_globals.models, &model, sizeof(model), realloc);

    model_import_from_file(&model, "../assets/models/monkey.obj");
    mempush(&render_globals.model_count, (void **)&render_globals.models, &model, sizeof(model), realloc);

    for (int model_index = 0; model_index < render_globals.model_count; model_index++)
    {
        struct model_data *model = render_globals.models + model_index;
            
        for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
        {
            struct model_mesh *mesh = model->meshes + mesh_index;

            if (!mesh->vertices)
                continue;

            // Create and bind the mesh's vertex array
            glGenVertexArrays(1, &mesh->vertex_array);
            glBindVertexArray(mesh->vertex_array);

            // Create, bind and fill the mesh's vertex buffer
            glGenBuffers(1, &mesh->vertex_buffer);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer);
            glBufferData(GL_ARRAY_BUFFER, mesh->vertex_count * sizeof(struct model_vertex), mesh->vertices, GL_STATIC_DRAW);

            // Describe the position attribute for each vertex in the mesh's vertex buffer
            GLuint position_location = glGetAttribLocation(render_globals.program, "position");
            glEnableVertexAttribArray(position_location);
            glVertexAttribPointer(
                position_location, 3, GL_FLOAT, GL_FALSE,
                sizeof(struct model_vertex),
                (const void *)offsetof(struct model_vertex, position));

            // Describe the normal attribute for each vertex in the mesh's vertex buffer
            GLuint normal_location = glGetAttribLocation(render_globals.program, "normal");
            glEnableVertexAttribArray(normal_location);
            glVertexAttribPointer(
                normal_location, 3, GL_FLOAT, GL_FALSE,
                sizeof(struct model_vertex),
                (const void *)offsetof(struct model_vertex, normal));

            // Describe the texcoord attribute for each vertex in the mesh's vertex buffer
            GLuint texcoord_location = glGetAttribLocation(render_globals.program, "texcoord");
            glEnableVertexAttribArray(texcoord_location);
            glVertexAttribPointer(
                texcoord_location, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct model_vertex),
                (const void *)offsetof(struct model_vertex, texcoord));

            // Describe the tangent attribute for each vertex in the mesh's vertex buffer
            GLuint tangent_location = glGetAttribLocation(render_globals.program, "tangent");
            glEnableVertexAttribArray(tangent_location);
            glVertexAttribPointer(
                tangent_location, 3, GL_FLOAT, GL_FALSE,
                sizeof(struct model_vertex),
                (const void *)offsetof(struct model_vertex, tangent));

            // Describe the bitangent attribute for each vertex in the mesh's vertex buffer
            GLuint bitangent_location = glGetAttribLocation(render_globals.program, "bitangent");
            glEnableVertexAttribArray(bitangent_location);
            glVertexAttribPointer(
                bitangent_location, 3, GL_FLOAT, GL_FALSE,
                sizeof(struct model_vertex),
                (const void *)offsetof(struct model_vertex, bitangent));
        }
    }
}

void render_dispose(void)
{
    // TODO
}

void render_handle_screen_resize(int width, int height)
{
    render_globals.camera.aspect_ratio = (float)width / (float)height;

    float inverse_aspect_ratio = (float)height / (float)width;
    render_globals.camera.vertical_fov = 2.0f * atanf(tanf(glm_rad(render_globals.camera.horizontal_fov) / 2.0f) * inverse_aspect_ratio);
}

void render_update(float delta_ticks)
{
    // Cycle through camera speed intervals when the tab key is pressed and released
    if (input_is_key_down(SDL_SCANCODE_TAB))
    {
        render_globals.tab_pressed = true;
    }
    else if (render_globals.tab_pressed)
    {
        render_globals.tab_pressed = false;

        if (render_globals.camera.movement_speed == 1.0f)
            render_globals.camera.movement_speed = 5.0f;
        else if (render_globals.camera.movement_speed == 5.0f)
            render_globals.camera.movement_speed = 10.0f;
        else if (render_globals.camera.movement_speed == 10.0f)
            render_globals.camera.movement_speed = 20.0f;
        else if (render_globals.camera.movement_speed == 20.0f)
            render_globals.camera.movement_speed = 100.0f;
        else
            render_globals.camera.movement_speed = 1.0f;
    }
    
    camera_update(&render_globals.camera, delta_ticks);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int model_index = 0; model_index < render_globals.model_count; model_index++)
    {
        if (model_index == render_globals.weapon_model_index)
            continue;
        
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);
        render_model(model_index, model_matrix);
    }

    if (render_globals.weapon_model_index != -1)
    {
        mat4 model_matrix;
        glm_mat4_identity(model_matrix);
        glm_scale_uni(model_matrix, 0.25f);
        glm_rotate(model_matrix, 1.4f, (vec3){0, 1, 0});
        glm_rotate(model_matrix, -0.05f, (vec3){0, 0, 1});
        glm_translate(model_matrix, (vec3){0.6f, -0.725f, 0.3f});

        mat4 inverted_view;
        glm_mat4_inv(render_globals.camera.view, inverted_view);
        glm_mat4_mul(inverted_view, model_matrix, model_matrix);

        render_model(render_globals.weapon_model_index, model_matrix);
    }
}

/* ---------- private code */

static void render_model(int model_index, mat4 model_matrix)
{
    struct model_data *model = render_globals.models + model_index;

    for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
    {
        struct model_mesh *mesh = model->meshes + mesh_index;

        // Bind the shader
        glUseProgram(render_globals.program);

        // Bind the camera position
        glUniform3fv(glGetUniformLocation(render_globals.program, "camera_position"), 1, (const GLfloat *)render_globals.camera.position);

        // Bind the camera model/view/projection matrices
        glUniformMatrix4fv(glGetUniformLocation(render_globals.program, "model"), 1, GL_FALSE, (const GLfloat *)model_matrix);
        glUniformMatrix4fv(glGetUniformLocation(render_globals.program, "view"), 1, GL_FALSE, (const GLfloat *)render_globals.camera.view);
        glUniformMatrix4fv(glGetUniformLocation(render_globals.program, "projection"), 1, GL_FALSE, (const GLfloat *)render_globals.camera.projection);

        // Bind the lighting uniforms
        glUniform1uiv(glGetUniformLocation(render_globals.program, "directional_light_count"), 1, (const GLuint[]){1});
        
        glUniform3fv(glGetUniformLocation(render_globals.program, "directional_lights[0].position"), 1, (const vec3){1.2f, 3.0f, 2.0f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "directional_lights[0].direction"), 1, (const vec3){-0.2f, -1.0f, -0.3f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "directional_lights[0].diffuse_color"), 1, (const vec3){0.8f, 0.2f, 0.1f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "directional_lights[0].ambient_color"), 1, (const vec3){0.05f, 0.05f, 0.05f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "directional_lights[0].specular_color"), 1, (const vec3){1.0f, 1.0f, 1.0f});

        glUniform1uiv(glGetUniformLocation(render_globals.program, "point_light_count"), 1, (const GLuint[]){4});
        
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[0].position"), 1, (const vec3){0.7f, 0.2f, 2.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[0].constant"), 1, (const GLfloat[]){1.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[0].linear"), 1, (const GLfloat[]){0.09f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[0].quadratic"), 1, (const GLfloat[]){0.032f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[0].diffuse_color"), 1, (const vec3){0.1f, 0.8f, 0.2f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[0].ambient_color"), 1, (const vec3){0.05f, 0.05f, 0.05f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[0].specular_color"), 1, (const vec3){1.0f, 1.0f, 1.0f});

        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[1].position"), 1, (const vec3){2.3f, -3.3f, -4.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[1].constant"), 1, (const GLfloat[]){1.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[1].linear"), 1, (const GLfloat[]){0.09f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[1].quadratic"), 1, (const GLfloat[]){0.032f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[1].diffuse_color"), 1, (const vec3){0.1f, 0.2f, 0.8f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[1].ambient_color"), 1, (const vec3){0.05f, 0.05f, 0.05f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[1].specular_color"), 1, (const vec3){1.0f, 1.0f, 1.0f});

        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[2].position"), 1, (const vec3){-4.0f, 2.0f, -12.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[2].constant"), 1, (const GLfloat[]){1.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[2].linear"), 1, (const GLfloat[]){0.09f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[2].quadratic"), 1, (const GLfloat[]){0.032f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[2].diffuse_color"), 1, (const vec3){0.8f, 0.8f, 0.8f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[2].ambient_color"), 1, (const vec3){0.05f, 0.05f, 0.05f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[2].specular_color"), 1, (const vec3){1.0f, 1.0f, 1.0f});

        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[3].position"), 1, (const vec3){0.0f, 0.0f, -3.0});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[3].constant"), 1, (const GLfloat[]){1.0f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[3].linear"), 1, (const GLfloat[]){0.09f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "point_lights[3].quadratic"), 1, (const GLfloat[]){0.032f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[3].diffuse_color"), 1, (const vec3){0.8f, 0.8f, 0.8f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[3].ambient_color"), 1, (const vec3){0.05f, 0.05f, 0.05f});
        glUniform3fv(glGetUniformLocation(render_globals.program, "point_lights[3].specular_color"), 1, (const vec3){1.0f, 1.0f, 1.0f});

        // Bind the material uniforms
        glUniform1fv(glGetUniformLocation(render_globals.program, "material.specular_amount"), 1, (const GLfloat[]){0.5f});
        glUniform1fv(glGetUniformLocation(render_globals.program, "material.specular_shininess"), 1, (const GLfloat[]){32});
        glUniform1fv(glGetUniformLocation(render_globals.program, "material.ambient_amount"), 1, (const GLfloat[]){0.1f});

        // Activate and bind the material texture(s)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_globals.diffuse_texture);
        glUniform1i(glGetUniformLocation(render_globals.program, "material.diffuse_texture"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, render_globals.specular_texture);
        glUniform1i(glGetUniformLocation(render_globals.program, "material.specular_texture"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, render_globals.normal_texture);
        glUniform1i(glGetUniformLocation(render_globals.program, "material.normal_texture"), 2);

        // Draw the geometry
        glBindVertexArray(mesh->vertex_array);

        for (int part_index = 0; part_index < mesh->part_count; part_index++)
        {
            struct model_mesh_part *part = mesh->parts + part_index;
            glDrawArrays(GL_TRIANGLES, part->vertex_index, part->vertex_count);
        }
    }
}

static GLuint render_compile_shader_source(
    GLenum shader_type,
    const char *shader_source)
{
    GLuint result = glCreateShader(shader_type);
    glShaderSource(result, 1, &shader_source, NULL);
    glCompileShader(result);

    GLint compile_status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compile_status);

    if (compile_status == GL_FALSE)
    {
        GLint maximum_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &maximum_log_length);

        GLsizei log_length;
        char log[maximum_log_length];
        glGetShaderInfoLog(result, maximum_log_length, &log_length, log);

        fprintf(stderr, "GLSL compile error: %s\n", log);
    }

    return result;
}

static GLuint render_load_and_compile_shader_file(
    GLenum shader_type,
    const char *file_path)
{
    FILE *stream = fopen(file_path, "r");

    fseek(stream, 0, SEEK_END);
    size_t file_size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    char file_data[file_size + 1];
    fread(file_data, file_size, 1, stream);
    file_data[file_size] = '\0';

    fclose(stream);

    return render_compile_shader_source(shader_type, file_data);
}

static GLuint render_load_shader_program(
    const char *vertex_shader_path,
    const char *fragment_shader_path)
{
    GLuint vertex_shader = render_load_and_compile_shader_file(GL_VERTEX_SHADER, vertex_shader_path);
    GLuint fragment_shader = render_load_and_compile_shader_file(GL_FRAGMENT_SHADER, fragment_shader_path);

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);

    glLinkProgram(program);
    glUseProgram(program);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

GLuint render_load_dds_file_as_texture2d(
    const char *file_path)
{
    struct dds_data dds;
    
    if (!dds_from_file(&dds, file_path))
    {
        fprintf(stderr, "ERROR: failed to load dds as texture2d\n");
        return 0;
    }

    GLenum format;

    switch (dds.header.fourcc)
    {
    case _dds_fourcc_dxt1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;

    case _dds_fourcc_dxt3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;

    case _dds_fourcc_dxt5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;

    default:
        fprintf(stderr, "ERROR: unsupported DDS format: %u\n", dds.header.fourcc);
        dds_dispose(&dds);
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    for (unsigned int
        block_size = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16,
        width = dds.header.width,
        height = dds.header.height,
        offset = 0,
        level = 0,
        size;
        
        size = ((width + 3) / 4) * ((height + 3) / 4) * block_size,
        (level < dds.header.mip_map_count) && (width || height);
        
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, dds.data + offset),
        width = width > 1 ? width / 2 : 1,
        height = height > 1 ? height / 2 : 1,
        offset += size,
        level++);

    dds_dispose(&dds);

    return texture_id;
}
