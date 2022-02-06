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
#include "obj.h"
#include "render.h"

/* ---------- private types */

struct render_camera
{
    float field_of_view;
    float aspect_ratio;
    float near_clip;
    float far_clip;
    vec3 position;
    vec3 target;
    vec3 up;
    mat4 mvp;
};

struct render_model
{
    int material_count;
    int mesh_count;
    struct render_material *materials;
    struct render_mesh *meshes;
};

struct render_material
{
    GLuint program;
};

struct render_mesh
{
    GLuint vertex_array;
    GLuint vertex_buffer;
    int part_count;
    struct render_mesh_part *parts;
};

struct render_mesh_part
{
    int material_index;
    GLint vertex_index;
    GLsizei vertex_count;
};

struct render_vertex
{
    vec3 position;
    vec3 normal;
    vec2 texcoord;
};

/* ---------- private variables */

struct
{
    struct render_camera camera;

    int model_count;
    struct render_model *models;

    GLuint program;
    GLuint diffuse_texture;
} render_globals;

/* ---------- private prototypes */

static void render_camera_initialize(void);
static void render_camera_update(float delta_time);

static GLuint render_compile_shader_source(GLenum shader_type, const char *shader_source);
static GLuint render_load_and_compile_shader_file(GLenum shader_type, const char *file_path);
static GLuint render_load_shader_program(const char *vertex_shader_path, const char *fragment_shader_path);
static GLuint render_load_dds_file_as_texture2d(const char *file_path);
static void render_load_obj_file(const char *file_path);

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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    render_camera_initialize();

    render_globals.program = render_load_shader_program("../assets/shaders/basic.vs", "../assets/shaders/basic.fs");
    render_globals.diffuse_texture = render_load_dds_file_as_texture2d("../assets/textures/asdf.dds");

    render_load_obj_file("../assets/models/cube_sphere.obj");
}

void render_dispose(void)
{
    // TODO
}

void render_handle_screen_resize(int width, int height)
{
    render_globals.camera.aspect_ratio = (float)width / (float)height;
}

void render_update(float delta_time)
{
    render_camera_update(delta_time);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    for (int model_index = 0; model_index < render_globals.model_count; model_index++)
    {
        struct render_model *model = render_globals.models + model_index;

        for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
        {
            struct render_mesh *mesh = model->meshes + mesh_index;

            // Bind the shader
            glUseProgram(render_globals.program);

            // Bind the camera model/view/projection matrices
            glUniformMatrix4fv(glGetUniformLocation(render_globals.program, "mvp"), 1, GL_FALSE, (const GLfloat *)render_globals.camera.mvp);

            // Activate and bind the texture(s)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, render_globals.diffuse_texture);

            // Register the texture unit(s) in the shader
            glUniform1i(glGetUniformLocation(render_globals.program, "diffuse_texture"), 0);

            // Bind the lighting uniforms
            glUniform1fv(glGetUniformLocation(render_globals.program, "ambient_amount"), 1, (const GLfloat[]){0.1f});
            glUniform3fv(glGetUniformLocation(render_globals.program, "light_position"), 1, (const GLfloat[]){1.2f, 1.0f, 2.0f});
            glUniform3fv(glGetUniformLocation(render_globals.program, "light_color"), 1, (const GLfloat[]){0.4f, 0.6, 0.8f});

            // Draw the geometry
            glBindVertexArray(mesh->vertex_array);

            for (int part_index = 0; part_index < mesh->part_count; part_index++)
            {
                struct render_mesh_part *part = mesh->parts + part_index;

                // Draw the geometry
                glDrawArrays(GL_TRIANGLES, part->vertex_index, part->vertex_count);
            }
        }
    }
}

/* ---------- private code */

static void render_camera_initialize(void)
{
    render_globals.camera.field_of_view = 90.0f;
    render_globals.camera.aspect_ratio = 1.0f;
    render_globals.camera.near_clip = 0.1f;
    render_globals.camera.far_clip = 1000.0f;
    memcpy(render_globals.camera.position, (vec3){1.0f, 1.0f, 2.0f}, sizeof(vec3));
    memcpy(render_globals.camera.target, (vec3){0.0f, 0.0f, 0.0f}, sizeof(vec3));
    memcpy(render_globals.camera.up, (vec3){0.0f, 1.0f, 0.0f}, sizeof(vec3));
}

static void render_camera_update(float delta_time)
{
    vec3 camera_reverse;
    glm_vec3_sub(render_globals.camera.position, render_globals.camera.target, camera_reverse);
    glm_vec3_normalize(camera_reverse);

    vec3 camera_right;
    glm_vec3_cross(render_globals.camera.up, camera_reverse, camera_right);
    glm_normalize(camera_right);

    vec3 camera_up;
    glm_vec3_cross(camera_reverse, camera_right, camera_up);

    vec3 move_amount = GLM_VEC3_ZERO_INIT;

    if (input_is_key_down(SDL_SCANCODE_S))
    {
        glm_vec3_add(move_amount, camera_reverse, move_amount);
    }
    else if (input_is_key_down(SDL_SCANCODE_W))
    {
        glm_vec3_sub(move_amount, camera_reverse, move_amount);
    }

    if (input_is_key_down(SDL_SCANCODE_D))
    {
        glm_vec3_add(move_amount, camera_right, move_amount);
    }
    else if (input_is_key_down(SDL_SCANCODE_A))
    {
        glm_vec3_sub(move_amount, camera_right, move_amount);
    }

    if (input_is_key_down(SDL_SCANCODE_R))
    {
        glm_vec3_add(move_amount, camera_up, move_amount);
    }
    else if (input_is_key_down(SDL_SCANCODE_F))
    {
        glm_vec3_sub(move_amount, camera_up, move_amount);
    }

    if (input_is_key_down(SDL_SCANCODE_LSHIFT))
    {
        glm_vec3_scale(move_amount, 2.0f, move_amount);
    }

    glm_vec3_scale(move_amount, 5.0f * delta_time, move_amount);

    glm_vec3_add(render_globals.camera.position, move_amount, render_globals.camera.position);
    glm_vec3_add(render_globals.camera.target, move_amount, render_globals.camera.target);

    mat4 model;
    glm_mat4_identity(model);

    mat4 view;
    glm_lookat(
        render_globals.camera.position,
        render_globals.camera.target,
        render_globals.camera.up,
        view);
    
    mat4 projection;
    glm_perspective(
        render_globals.camera.field_of_view / 2.0f,
        render_globals.camera.aspect_ratio,
        render_globals.camera.near_clip,
        render_globals.camera.far_clip,
        projection);
    
    glm_mat4_mulN(
        (mat4 *[]){&projection, &view, &model}, 3,
        render_globals.camera.mvp);
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

static void render_load_obj_file(
    const char *file_path)
{
    // Read the OBJ file from disk
    struct obj obj;
    obj_from_file(&obj, file_path);

    // Initialize a new render model
    struct render_model model;
    memset(&model, 0, sizeof(model));

    int v_count_total = 0;
    int vn_count_total = 0;
    int vt_count_total = 0;

    for (int o_index = 0; o_index < obj.o_count; o_index++)
    {
        struct obj_o *o = obj.o + o_index;

        struct render_mesh mesh;
        memset(&mesh, 0, sizeof(mesh));

        int vertex_count = 0;
        struct render_vertex *vertices = NULL;

        for (int g_index = 0; g_index < o->g_count; g_index++)
        {
            struct obj_g *g = o->g + g_index;

            struct render_mesh_part part =
            {
                .material_index = -1, // TODO
                .vertex_index = vertex_count,
                .vertex_count = 0,
            };

            for (int f_index = 0; f_index < g->f_count; f_index++)
            {
                struct obj_f *f = g->f + f_index;
                assert(f->count == 3);
                
                for (int i = 0; i < f->count; i++)
                {
                    struct render_vertex vertex;
                    memset(&vertex, 0, sizeof(vertex));

                    int v_index = (f->v_indices[i] ? f->v_indices[i] - v_count_total : 0) - 1;
                    if (v_index >= 0 && v_index < o->v_count)
                    {
                        struct obj_v *v = o->v + v_index;
                        memcpy(vertex.position, (vec3){v->x, v->y, v->z}, sizeof(vec3));
                    }

                    int vn_index = (f->vn_indices[i] ? f->vn_indices[i] - vn_count_total : 0) - 1;
                    if (vn_index >= 0 && vn_index < o->vn_count)
                    {
                        struct obj_vn *vn = o->vn + vn_index;
                        memcpy(vertex.normal, (vec3){vn->i, vn->j, vn->k}, sizeof(vec3));
                    }

                    int vt_index = (f->vt_indices[i] ? f->vt_indices[i] - vt_count_total : 0) - 1;
                    if (vt_index >= 0 && vt_index < o->vt_count)
                    {
                        struct obj_vt *vt = o->vt + vt_index;
                        memcpy(vertex.texcoord, (vec2){vt->u, vt->v}, sizeof(vec2));
                    }

                    // Add the vertex to the vertices of the mesh
                    mempush(&vertex_count, (void **)&vertices, &vertex, sizeof(vertex), realloc);
                }

                // Increase the number of vertices in the mesh part by the number of vertices in the face
                part.vertex_count += f->count;
            }

            // Add the mesh part to the mesh
            mempush(&mesh.part_count, (void **)&mesh.parts, &part, sizeof(part), realloc);
        }

        // Create and bind the mesh's vertex array
        glGenVertexArrays(1, &mesh.vertex_array);
        glBindVertexArray(mesh.vertex_array);

        // Create, bind and fill the mesh's vertex buffer
        glGenBuffers(1, &mesh.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct render_vertex), vertices, GL_STATIC_DRAW);

        // Describe the position attribute for each vertex in the mesh's vertex buffer
        GLuint position_location = glGetAttribLocation(render_globals.program, "position");
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(
            position_location, 3, GL_FLOAT, GL_FALSE,
            sizeof(struct render_vertex),
            (const void *)offsetof(struct render_vertex, position));

        // Describe the normal attribute for each vertex in the mesh's vertex buffer
        GLuint normal_location = glGetAttribLocation(render_globals.program, "normal");
        glEnableVertexAttribArray(normal_location);
        glVertexAttribPointer(
            normal_location, 3, GL_FLOAT, GL_FALSE,
            sizeof(struct render_vertex),
            (const void *)offsetof(struct render_vertex, normal));

        // Describe the texcoord attribute for each vertex in the mesh's vertex buffer
        GLuint texcoord_location = glGetAttribLocation(render_globals.program, "texcoord");
        glEnableVertexAttribArray(texcoord_location);
        glVertexAttribPointer(
            texcoord_location, 2, GL_FLOAT, GL_FALSE,
            sizeof(struct render_vertex),
            (const void *)offsetof(struct render_vertex, texcoord));
        
        // TODO: Create, bind and fill the mesh's element buffer

        // Add the mesh to the model's meshes
        mempush(&model.mesh_count, (void **)&model.meshes, &mesh, sizeof(mesh), realloc);
        
        // Increase the total number of vertices for resolving the indices of the next obj_f
        v_count_total += o->v_count;
        vn_count_total += o->vn_count;
        vt_count_total += o->vt_count;
    }

    // Add the model to the global models
    mempush(&render_globals.model_count, (void **)&render_globals.models, &model, sizeof(model), realloc);

    // Dispose the obj model data
    obj_dispose(&obj);
}
