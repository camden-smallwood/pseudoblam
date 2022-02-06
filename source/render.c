#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <GL/glew.h>
#include <cglm/cglm.h>

#include "common.h"
#include "dds.h"
#include "obj.h"
#include "render.h"

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
    int material_index;
    GLint vertex_index;
    GLsizei vertex_count;
};

struct render_vertex
{
    vec3 position;
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

    render_load_obj_file("../assets/models/monkey.obj");
}

void render_dispose(void)
{
    // TODO
}

void render_update(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Create the camera model/view/projection matrix
    mat4 model, view, projection, mvp;
    glm_mat4_identity(model);
    glm_lookat(render_globals.camera.position, render_globals.camera.target, render_globals.camera.up, view);
    glm_perspective(render_globals.camera.field_of_view, 1280.0f / 720.0f, render_globals.camera.near_clip, render_globals.camera.far_clip, projection);
    glm_mat4_mulN((mat4 *[]){&projection, &view, &model}, 3, mvp);

    for (int model_index = 0; model_index < render_globals.model_count; model_index++)
    {
        struct render_model *model = render_globals.models + model_index;

        for (int mesh_index = 0; mesh_index < model->mesh_count; mesh_index++)
        {
            struct render_mesh *mesh = model->meshes + mesh_index;

            // Bind the shader
            glUseProgram(render_globals.program);

            // Bind the camera model/view/projection matrix
            glUniformMatrix4fv(glGetUniformLocation(render_globals.program, "mvp"), 1, GL_FALSE, (const GLfloat *)mvp);

            // Activate and bind the texture(s)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, render_globals.diffuse_texture);

            // Register the texture unit(s) in the shader
            glUniform1i(glGetUniformLocation(render_globals.program, "diffuse_texture"), 0);

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

                    int v_index = f->v_indices[i] - 1;
                    struct obj_v *v = o->v + v_index;
                    memcpy(vertex.position, (vec3){v->x, v->y, v->z}, sizeof(vec3));

                    int vt_index = f->vt_indices[i] - 1;
                    if (vt_index >= 0 && vt_index < o->vt_count)
                    {
                        struct obj_vt *vt = o->vt + vt_index;
                        memcpy(vertex.texcoord, (vec2){vt->u, vt->v}, sizeof(vec2));
                    }

                    mempush(&vertex_count, (void **)&vertices, &vertex, sizeof(vertex), realloc);
                }

                part.vertex_count += f->count;
            }

            mempush(&mesh.part_count, (void **)&mesh.parts, &part, sizeof(part), realloc);
        }

        glGenVertexArrays(1, &mesh.vertex_array);
        glBindVertexArray(mesh.vertex_array);

        glGenBuffers(1, &mesh.vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(struct render_vertex), vertices, GL_STATIC_DRAW);

        GLuint position_location = glGetAttribLocation(render_globals.program, "position");
        glEnableVertexAttribArray(position_location);
        glVertexAttribPointer(position_location, 3, GL_FLOAT, GL_FALSE, 20, (const void *)0);

        GLuint texcoord_location = glGetAttribLocation(render_globals.program, "texcoord");
        glEnableVertexAttribArray(texcoord_location);
        glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, 20, (const void *)12);

        mempush(&model.mesh_count, (void **)&model.meshes, &mesh, sizeof(mesh), realloc);
    }

    mempush(&render_globals.model_count, (void **)&render_globals.models, &model, sizeof(model), realloc);

    obj_dispose(&obj);
}
