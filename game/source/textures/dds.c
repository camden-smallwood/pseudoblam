#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "textures/dds.h"
#include "rasterizer/rasterizer_textures.h"

/* ---------- public code */

void dds_dispose(struct dds_data *dds)
{
    assert(dds);
    free(dds->data);
}

void dds_from_file(struct dds_data *dds, const char *file_path)
{
    assert(dds);
    assert(file_path);
    
    FILE *stream = fopen(file_path, "rb");

    if (stream == NULL)
    {
        fprintf(stderr, "ERROR: \"%s\" could not be opened\n", file_path);
        exit(EXIT_FAILURE);
    }

    fread(&dds->header, sizeof(dds->header), 1, stream);

    if (strncmp(dds->header.filecode, "DDS ", 4) != 0)
    {
        fprintf(stderr, "ERROR: \"%s\" is not a valid DDS file\n", file_path);
        fclose(stream);
        exit(EXIT_FAILURE);
    }

    unsigned int data_size = dds->header.mip_map_count > 1
        ? dds->header.linear_size * 2
        : dds->header.linear_size;
    
    assert(dds->data = malloc(data_size));

    fread(dds->data, 1, data_size, stream);
    fclose(stream);
}

int dds_import_file_as_texture2d(
    const char *file_path)
{
    assert(file_path);

    struct dds_data dds;
    dds_from_file(&dds, file_path);

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

    int texture_index = texture_allocate(_texture_type_2d);
    struct texture_data *texture = texture_get_data(texture_index);

    texture->width = dds.header.width;
    texture->height = dds.header.height;

    glGenTextures(1, &texture->id);

    glBindTexture(GL_TEXTURE_2D, texture->id);
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

    return texture_index;
}
