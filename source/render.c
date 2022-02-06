#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <SDL.h>
#include <GL/glew.h>
#include <cglm/cglm.h>
#include "dds.h"
#include "obj.h"
#include "render.h"
#include "common.h"

/* ---------- private types */

struct render_camera
{
    vec3 position;
    vec3 target;
    vec3 up;
    float near_clip;
    float far_clip;
    float field_of_view;
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
    GLuint element_buffer;
};

/* ---------- private variables */

struct
{
    struct render_camera camera;

    int model_count;
    struct render_model *models;

    GLuint program;
    GLuint diffuse_texture;
    GLuint vertex_array;
    GLuint vertex_buffer;
} render_globals;

/* ---------- private prototypes */

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

    memcpy(render_globals.camera.position, (vec3){3.0f, 3.0f, 3.0f}, sizeof(vec3));
    memcpy(render_globals.camera.target, (vec3){0.0f, 0.0f, 0.0f}, sizeof(vec3));
    memcpy(render_globals.camera.up, (vec3){0.0f, 1.0f, 0.0f}, sizeof(vec3));
    render_globals.camera.near_clip = 0.1f;
    render_globals.camera.far_clip = 1000.0f;
    render_globals.camera.field_of_view = 90.0f;

    render_globals.program = render_load_shader_program("../assets/shaders/basic.vs", "../assets/shaders/basic.fs");
    render_globals.diffuse_texture = render_load_dds_file_as_texture2d("../assets/textures/asdf.dds");

    render_load_obj_file("../assets/models/cube.obj");

    const GLfloat vertices[] =
    {
        // Bottom
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,

        // Top
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f,

        // Front
        -1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,

        // Back
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 1.0f,

        // Left
        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 1.0f, 0.0f,

        // Right
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    };

    glGenVertexArrays(1, &render_globals.vertex_array);
    glBindVertexArray(render_globals.vertex_array);

    glGenBuffers(1, &render_globals.vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, render_globals.vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint position_location = glGetAttribLocation(render_globals.program, "position");
    glEnableVertexAttribArray(position_location);
    glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 32, (const void *)0);

    GLuint normal_location = glGetAttribLocation(render_globals.program, "normal");
    glEnableVertexAttribArray(normal_location);
    glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 32, (const void *)12);

    GLuint texcoord_location = glGetAttribLocation(render_globals.program, "texcoord");
    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, 32, (const void *)24);
}

void render_dispose(void)
{
    // TODO
}

void render_update(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the shader
    glUseProgram(render_globals.program);

    // Create and bind the camera model/view/projection matrix
    mat4 model, view, projection, mvp;
    glm_mat4_identity(model);
    glm_lookat(render_globals.camera.position, render_globals.camera.target, render_globals.camera.up, view);
    glm_perspective(render_globals.camera.field_of_view, 1280.0f / 720.0f, render_globals.camera.near_clip, render_globals.camera.far_clip, projection);
    glm_mat4_mulN((mat4 *[]){&projection, &view, &model}, 3, mvp);
    glUniformMatrix4fv(glGetUniformLocation(render_globals.program, "mvp"), 1, GL_FALSE, (const GLfloat *)mvp);

    // Activate and bind the texture(s)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render_globals.diffuse_texture);

    // Register the texture unit(s) in the shader
    glUniform1i(glGetUniformLocation(render_globals.program, "diffuse_texture"), 0);

    // Draw the geometry
    glBindVertexArray(render_globals.vertex_array);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

/* ---------- private code */

static GLuint render_compile_shader_source(
    GLenum shader_type,
    const char *shader_source)
{
    GLuint result = glCreateShader(shader_type);
    glShaderSource(result, 1, &shader_source, NULL);
    glCompileShader(result);

    GLint vs_status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &vs_status);

    if (vs_status == GL_FALSE)
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

    unsigned int format;

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
        level = 0;
        
        (level < dds.header.mip_map_count) && (width || height);
        
        width = width > 1 ? width / 2 : 1,
        height = height > 1 ? height / 2 : 1,
        level++)
    {
        unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, dds.data + offset);
        offset += size;
    }

    dds_dispose(&dds);

    return texture_id;
}

static void render_load_obj_file(
    const char *file_path)
{
    struct obj obj;
    obj_from_file(&obj, file_path);

    struct render_model model;
    memset(&model, 0, sizeof(model));

    for (int o_index = 0; o_index < obj.o_count; o_index++)
    {
        struct obj_o *o = obj.o + o_index;

        struct render_mesh mesh;
        memset(&mesh, 0, sizeof(mesh));

        // TODO: create vertex buffer

        for (int g_index = 0; g_index < o->g_count; g_index++)
        {
            struct obj_g *g = o->g + g_index;

            struct render_mesh_part part;
            memset(&part, 0, sizeof(part));

            // TODO: create element buffer

            for (int f_index = 0; f_index < g->f_count; f_index++)
            {
                struct obj_f *f = g->f + f_index;
                assert(f);

                // TODO: populate element buffer
            }

            mempush(&mesh.part_count, (void **)&mesh.parts, &part, sizeof(part), realloc);
        }

        mempush(&model.mesh_count, (void **)&model.meshes, &mesh, sizeof(mesh), realloc);
    }

    mempush(&render_globals.model_count, (void **)&render_globals.models, &model, sizeof(model), realloc);

    obj_dispose(&obj);
}
