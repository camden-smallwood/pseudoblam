#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

int main()
{
    const char *path = "../assets/models/cube.glb";

    cgltf_options options;
    memset(&options, 0, sizeof(options));

    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result == cgltf_result_success)
        result = cgltf_load_buffers(&options, data, path);
    
    if (result == cgltf_result_success)
        result = cgltf_validate(data);
    
    printf("result: %d\n", result);

    if (result == cgltf_result_success)
    {
        printf("file_type: %u\n", data->file_type);
        printf("meshes_count: %lu\n", data->meshes_count);
    }

    cgltf_free(data);

    return 0;
}
