#pragma once
#include <stdbool.h>

/* ---------- constants */

enum dds_fourcc
{
    _dds_fourcc_dxt1 = '1TXD',
    _dds_fourcc_dxt3 = '3TXD',
    _dds_fourcc_dxt5 = '5TXD',
};

/* ---------- structures */

struct dds_header
{
    char filecode[4];
    unsigned int : 32;
    unsigned int : 32;
    unsigned int height;
    unsigned int width;
    unsigned int linear_size;
    unsigned int : 32;
    unsigned int mip_map_count;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int fourcc;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
    unsigned int : 32;
};

struct dds_data
{
    struct dds_header header;
    char *data;
};

/* ---------- prototypes/dds.c */

void dds_dispose(struct dds_data *dds);
bool dds_from_file(struct dds_data *dds, const char *file_path);

unsigned int dds_import_file_as_texture2d(const char *file_path);
