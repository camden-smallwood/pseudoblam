#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "obj.h"

/* ---------- private prototypes */

static inline void *obj_push_sized(int *out_count, void **out_address, void *address, size_t size);

static inline char *obj_internalize_string(struct obj *obj, const char *string);

static inline struct obj_o *obj_get_or_start_new_o(struct obj_o **op);
static inline struct obj_g *obj_get_or_start_new_g(struct obj_g **gp);

static inline void obj_finish_o(struct obj *obj, struct obj_o *o, struct obj_g **gp);
static inline void obj_finish_g(struct obj_o *o, struct obj_g *g);

static inline struct obj_o *obj_finish_and_start_new_o(struct obj *obj, struct obj_o **op, struct obj_g **gp);
static inline struct obj_g *obj_finish_and_start_new_g(struct obj_o *o, struct obj_g **gp);

static inline char *obj_parse_next_token_optional(const char *delimiter);
static inline char *obj_parse_next_token_required(const char *delimiter);

static inline float obj_parse_next_float_required(const char *delimiter);
static inline float obj_parse_next_float_optional(const char *delimiter, float default_value);

static void obj_parse_mtllib(struct obj *obj, struct obj_o **, struct obj_g **);
static void obj_parse_o(struct obj *obj, struct obj_o **op, struct obj_g **gp);
static void obj_parse_v(struct obj *, struct obj_o **op, struct obj_g **);
static void obj_parse_vt(struct obj *, struct obj_o **op, struct obj_g **);
static void obj_parse_vn(struct obj *, struct obj_o **op, struct obj_g **);
static void obj_parse_g(struct obj *, struct obj_o **op, struct obj_g **gp);
static void obj_parse_s(struct obj *, struct obj_o **, struct obj_g **gp);
static void obj_parse_usemtl(struct obj *, struct obj_o **, struct obj_g **gp);
static void obj_parse_f(struct obj *, struct obj_o **op, struct obj_g **gp);

static void obj_parse_f_chunk(struct obj_o *o, struct obj_f *f, char *token);

/* ---------- private constants */

struct
{
    const char *token;
    void(*parse)(struct obj *obj, struct obj_o **op, struct obj_g **gp);
}
static const obj_parsers[] =
{
    { "mtllib", obj_parse_mtllib },
    { "o", obj_parse_o },
    { "v", obj_parse_v },
    { "vt", obj_parse_vt },
    { "vn", obj_parse_vn },
    { "g", obj_parse_g },
    { "s", obj_parse_s },
    { "usemtl", obj_parse_usemtl },
    { "f", obj_parse_f },
};

static const int obj_parser_count = sizeof(obj_parsers) / sizeof(obj_parsers[0]);

/* ---------- public code */

void obj_read(struct obj *obj, const char *path)
{
    assert(obj);
    assert(path);

    memset(obj, 0, sizeof(*obj));

    struct obj_o *o = NULL;
    struct obj_g *g = NULL;

    FILE *stream = fopen(path, "r");

    size_t line_length = 0;
    char *line = NULL;

    while (getline(&line, &line_length, stream) >= 0)
    {
        line[strcspn(line, "\r\n")] = '\0';
        char *token = strtok(line, " ");
        
        if (!token || strcmp(token, "#") == 0)
            continue;

        for (int parser_index = 0; parser_index < obj_parser_count; parser_index++)
        {
            if (strcmp(token, obj_parsers[parser_index].token) == 0)
            {
                obj_parsers[parser_index].parse(obj, &o, &g);
                break;
            }
        }
        
        if (strtok(NULL, " "))
            fprintf(stderr, "unhandled token: %s\n", token);
    }

    free(line);

    if (o)
    {
        obj_finish_o(obj, o, &g);
        free(o);
    }

    fclose(stream);
}

void obj_dispose(struct obj *obj)
{
    assert(obj);
    
    for (int mtllib_index = 0; mtllib_index < obj->mtllib_count; mtllib_index++)
    {
        struct obj_mtllib *mtllib = obj->mtllib + mtllib_index;

        free(mtllib->mtl);
    }
    
    for (int o_index = 0; o_index < obj->o_count; o_index++)
    {
        struct obj_o *o = obj->o + o_index;

        for (int g_index = 0; g_index < o->g_count; g_index++)
        {
            struct obj_g *g = o->g + g_index;

            for (int f_index = 0; f_index < g->f_count; f_index++)
            {
                struct obj_f *f = g->f + f_index;

                free(f->v_indices);
                free(f->vt_indices);
                free(f->vn_indices);
            }

            free(g->f);
        }

        free(o->v);
        free(o->vt);
        free(o->vn);
        free(o->g);
    }

    free(obj->string_offsets);
    free(obj->string_buffer);
    free(obj->mtllib);
    free(obj->o);
}

/* ---------- private code */

static inline void *obj_push_sized(int *out_count, void **out_address, void *address, size_t size)
{
    int index = (*out_count)++;
    assert(index < *out_count);

    *out_address = realloc(*out_address, *out_count * size);
    assert(*out_address);

    return memcpy(((char *)*out_address) + (index * size), address, size);
}

static inline char *obj_internalize_string(struct obj *obj, const char *string)
{
    int string_index = -1;

    for (int i = 0; i < obj->string_count; i++)
    {
        size_t string_offset = obj->string_offsets[i];

        if (strcmp(string, obj->string_buffer + string_offset) == 0)
        {
            string_index = i;
            break;
        }
    }

    if (string_index == -1)
    {
        size_t string_offset = obj->string_buffer_size;
        size_t string_size = strlen(string) + 1;

        assert(obj->string_buffer_size + string_size > obj->string_buffer_size);
        obj->string_buffer_size += string_size;
        assert(obj->string_buffer = realloc(obj->string_buffer, obj->string_buffer_size));
        memcpy(obj->string_buffer + string_offset, string, string_size);

        string_index = obj->string_count;
        obj_push_sized(&obj->string_count, (void **)&obj->string_offsets, &string_offset, sizeof(string_offset));
    }

    return obj->string_buffer + obj->string_offsets[string_index];
}

static inline struct obj_o *obj_get_or_start_new_o(struct obj_o **op)
{
    struct obj_o *o = *op;

    if (!o)
    {
        o = *op = calloc(1, sizeof(*o));
        assert(o);
    }

    return o;
}

static inline struct obj_g *obj_get_or_start_new_g(struct obj_g **gp)
{
    struct obj_g *g = *gp;

    if (!g)
    {
        g = *gp = calloc(1, sizeof(*g));
        assert(g);
    }

    return g;
}

static inline void obj_finish_o(struct obj *obj, struct obj_o *o, struct obj_g **gp)
{
    struct obj_g *g = *gp;

    if (g)
    {
        obj_finish_g(o, g);
        free(g);
        *gp = NULL;
    }

    obj_push_sized(&obj->o_count, (void **)&obj->o, o, sizeof(*o));
}

static inline void obj_finish_g(struct obj_o *o, struct obj_g *g)
{
    obj_push_sized(&o->g_count, (void **)&o->g, g, sizeof(*g));
}

static inline struct obj_o *obj_finish_and_start_new_o(struct obj *obj, struct obj_o **op, struct obj_g **gp)
{
    struct obj_o *o = *op;

    if (!o)
        o = obj_get_or_start_new_o(op);
    else
        obj_finish_o(obj, o, gp);

    return memset(o, 0, sizeof(*o));
}

static inline struct obj_g *obj_finish_and_start_new_g(struct obj_o *o, struct obj_g **gp)
{
    struct obj_g *g = *gp;

    if (!g)
        g = obj_get_or_start_new_g(gp);
    else
        obj_finish_g(o, g);

    return memset(g, 0, sizeof(*g));
}

static inline char *obj_parse_next_token_optional(const char *delimiter)
{
    return strtok(NULL, delimiter);
}

static inline char *obj_parse_next_token_required(const char *delimiter)
{
    char *string = obj_parse_next_token_optional(delimiter);
    assert(string);

    return string;
}

static inline float obj_parse_next_float_required(const char *delimiter)
{
    char *float_string = obj_parse_next_token_required(delimiter);

    return atof(float_string);
}

static inline float obj_parse_next_float_optional(const char *delimiter, float default_value)
{
    char *float_string = obj_parse_next_token_optional(delimiter);

    return float_string ? atof(float_string) : default_value;
}

static void obj_parse_mtllib(struct obj *obj, struct obj_o **, struct obj_g **)
{
    char *mtllib_path = obj_parse_next_token_required(" ");

    for (int mtllib_index = 0; mtllib_index < obj->mtllib_count; mtllib_index++)
        if (strcmp(mtllib_path, obj->mtllib[mtllib_index].path) == 0)
            return;

    char *path = obj_internalize_string(obj, mtllib_path);

    int mtl_count = 0;
    struct obj_mtl *mtl = NULL;

    //
    // TODO: load mtl file here (if available)
    //

    struct obj_mtllib mtllib = { path, mtl, mtl_count };

    obj_push_sized(&obj->mtllib_count, (void **)&obj->mtllib, &mtllib, sizeof(mtllib));
}

static void obj_parse_o(struct obj *obj, struct obj_o **op, struct obj_g **gp)
{
    struct obj_o *o = obj_finish_and_start_new_o(obj, op, gp);
    
    o->name = obj_internalize_string(obj, obj_parse_next_token_required(" "));
}

static void obj_parse_v(struct obj *, struct obj_o **op, struct obj_g **)
{
    struct obj_o *o = obj_get_or_start_new_o(op);

    float x = obj_parse_next_float_required(" ");
    float y = obj_parse_next_float_required(" ");
    float z = obj_parse_next_float_required(" ");
    float w = obj_parse_next_float_optional(" ", 1.0f);

    struct obj_v v = { x, y, z, w };
    
    obj_push_sized(&o->v_count, (void **)&o->v, &v, sizeof(v));
}

static void obj_parse_vt(struct obj *, struct obj_o **op, struct obj_g **)
{
    struct obj_o *o = obj_get_or_start_new_o(op);

    float u = obj_parse_next_float_required(" ");
    float v = obj_parse_next_float_required(" ");
    float w = obj_parse_next_float_optional(" ", 0.0f);
    
    struct obj_vt vt = { u, v, w };

    obj_push_sized(&o->vt_count, (void **)&o->vt, &vt, sizeof(vt));
}

static void obj_parse_vn(struct obj *, struct obj_o **op, struct obj_g **)
{
    struct obj_o *o = obj_get_or_start_new_o(op);

    float i = obj_parse_next_float_required(" ");
    float j = obj_parse_next_float_required(" ");
    float k = obj_parse_next_float_required(" ");

    struct obj_vn vn = { i, j, k };

    obj_push_sized(&o->vn_count, (void **)&o->vn, &vn, sizeof(vn));
}

static void obj_parse_g(struct obj *obj, struct obj_o **op, struct obj_g **gp)
{
    struct obj_o *o = obj_get_or_start_new_o(op);
    struct obj_g *g = obj_finish_and_start_new_g(o, gp);

    g->name = obj_internalize_string(obj, obj_parse_next_token_required(" "));
}

static void obj_parse_s(struct obj *, struct obj_o **, struct obj_g **gp)
{
    struct obj_g *g = obj_get_or_start_new_g(gp);

    char *token = obj_parse_next_token_required(" ");
    
    if (strcmp(token, "off") == 0)
    {
        g->s = 0;
    }
    else
    {
        size_t token_length = strlen(token);

        for (size_t i = 0; i < token_length; i++)
        {
            if (!isnumber(token[i]))
            {
                fprintf(stderr, "invalid 's' token: %s\n", token);
                return;
            }
        }

        g->s = atoi(token);
    }
}

static void obj_parse_usemtl(struct obj *obj, struct obj_o **, struct obj_g **gp)
{
    struct obj_g *g = obj_get_or_start_new_g(gp);
    
    g->usemtl = obj_internalize_string(obj, obj_parse_next_token_required(" "));
}

static void obj_parse_f(struct obj *, struct obj_o **op, struct obj_g **gp)
{
    struct obj_o *o = obj_get_or_start_new_o(op);
    struct obj_g *g = obj_get_or_start_new_g(gp);

    struct obj_f f = { 0, NULL, NULL, NULL };
    
    for (char *token = NULL,
        *next_token = strtok(NULL, "");
        
        token = strtok(next_token, " "),
        next_token = strtok(NULL, ""),
        token != NULL;

        obj_parse_f_chunk(o, &f, token));

    obj_push_sized(&g->f_count, (void **)&g->f, &f, sizeof(f));
}

static void obj_parse_f_chunk(struct obj_o *o, struct obj_f *f, char *token)
{
    char *v_index_string = strtok(token, "/");
    assert(v_index_string);
    int v_index = atoi(v_index_string);
    assert(v_index > 0 && v_index <= o->v_count);

    char *vt_index_string = strtok(NULL, "/");
    int vt_index = 0;
    if (vt_index_string && strlen(vt_index_string))
    {
        vt_index = atoi(vt_index_string);
        assert(vt_index > 0 && vt_index <= o->vt_count);
    }

    char *vn_index_string = strtok(NULL, " ");
    int vn_index = 0;
    if (vn_index_string && strlen(vn_index_string))
    {
        vn_index = atoi(vn_index_string);
        assert(vn_index > 0 && vn_index <= o->vn_count);
    }

    int index = f->count++;
    assert(index < f->count);
    
    f->v_indices = realloc(f->v_indices, f->count * sizeof(int));
    assert(f->v_indices);
    f->v_indices[index] = v_index;

    f->vt_indices = realloc(f->vt_indices, f->count * sizeof(int));
    assert(f->vt_indices);
    f->vt_indices[index] = vt_index;

    f->vn_indices = realloc(f->vn_indices, f->count * sizeof(int));
    assert(f->vn_indices);
    f->vn_indices[index] = vn_index;
}
