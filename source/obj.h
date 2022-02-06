#pragma once
#include <stdbool.h>
#include <stddef.h>

struct obj
{
    int mtllib_count;
    int o_count;
    int string_count;
    int tracked_address_count;

    struct obj_mtllib *mtllib;
    struct obj_o *o;
    
    size_t string_buffer_size;
    char *string_buffer;

    size_t *string_offsets;

    void **tracked_addresses;
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

void obj_dispose(struct obj *obj);
bool obj_from_file(struct obj *obj, const char *file_path);
