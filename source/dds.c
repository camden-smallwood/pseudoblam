#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dds.h"

/* ---------- public code */

void dds_dispose(struct dds_data *dds)
{
    assert(dds);
    free(dds->data);
}

bool dds_from_file(struct dds_data *dds, const char *file_path)
{
    assert(dds);
    assert(file_path);
    
    FILE *stream = fopen(file_path, "rb");

    if (stream == NULL)
    {
        fprintf(stderr, "ERROR: \"%s\" could not be opened\n", file_path);
        return false;
    }

    fread(&dds->header, sizeof(dds->header), 1, stream);

    if (strncmp(dds->header.filecode, "DDS ", 4) != 0)
    {
        fprintf(stderr, "ERROR: \"%s\" is not a valid DDS file\n", file_path);
        fclose(stream);
        return false;
    }

    unsigned int data_size = dds->header.mip_map_count > 1
        ? dds->header.linear_size * 2
        : dds->header.linear_size;
    
    assert(dds->data = malloc(data_size));

    fread(dds->data, 1, data_size, stream);
    fclose(stream);
    
    return true;
}
