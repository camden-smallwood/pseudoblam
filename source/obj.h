#pragma once
#include <stddef.h>

struct obj
{
    int string_count;
    size_t *string_offsets;
    size_t string_buffer_size;
    char *string_buffer;
    int mtllib_count;
    int o_count;
    struct obj_mtllib *mtllib;
    struct obj_o *o;
};

struct obj_mtllib
{
    char *path;
    struct obj_mtl *mtl;
    int mtl_count;
};

struct obj_mtl
{
    char *name;
    float Ns;
    float Ka[3];
    float Kd[3];
    float Ks[3];
    float Ke[3];
    float Ni;
    float d;
    float illum;
    char *map_Kd;
};

struct obj_o
{
    char *name;
    int v_count;
    int vt_count;
    int vn_count;
    int g_count;
    struct obj_v *v;
    struct obj_vt *vt;
    struct obj_vn *vn;
    struct obj_g *g;
};

struct obj_v
{
    float x;
    float y;
    float z;
    float w;
};

struct obj_vt
{
    float u;
    float v;
    float w;
};

struct obj_vn
{
    float i;
    float j;
    float k;
};

struct obj_g
{
    char *name;
    char *usemtl;
    int s;
    int f_count;
    struct obj_f *f;
};

struct obj_f
{
    int count;
    int *v_indices;
    int *vt_indices;
    int *vn_indices;
};

void obj_read(struct obj *obj, const char *path);
void obj_dispose(struct obj *obj);
