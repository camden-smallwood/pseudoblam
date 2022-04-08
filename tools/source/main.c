#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include "common/common.h"
#include "commands/commands.h"

const struct command_parameter_definition compile_model_parameters[] =
{
    { "path", _command_parameter_string, 0 },
};

static int compile_model_execute(int argc, const char **argv);

static const struct command_definition command_definitions[] =
{
    {
        "compile model",
        "Compiles a model from a source file.",
        NUMBER_OF(compile_model_parameters),
        compile_model_parameters,
        compile_model_execute,
    }
};

enum
{
    NUMBER_OF_COMMAND_DEFINITIONS = NUMBER_OF(command_definitions),
};

int main(int argc, const char **argv)
{
    argc--;
    argv++;

    if (argc == 0)
    {
        // TODO: run help command
        puts("TODO: run help command");
        return 0;
    }

    const char *command_name = argv[0];
    
    argc--;
    argv++;

    for (int i = 0; i < NUMBER_OF_COMMAND_DEFINITIONS; i++)
    {
        const struct command_definition *command = command_definitions + i;

        if (strcmp(command_name, command->name) == 0)
            return command->execute(argc, argv);
    }

    char *message = NULL;
    asprintf(&message, "%s", command_name);

    for (int i = 0; i < argc; i++)
    {
        const char *arg = argv[i];
        const char *arg_whitespace = string_find_whitespace(arg);
        asprintf(&message, arg_whitespace ? "%s \"%s\"" : "%s %s", message, arg);
    }

    fprintf(stderr, "ERROR: invalid command with %i args: %s\n", argc, message);
    free(message);
    exit(EXIT_FAILURE);
}

static int compile_model_execute(int argc, const char **argv)
{
    assert(argc == 1);

    const char *path = argv[0];

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

    //
    // TODO: generate model from glTF data
    //

    cgltf_free(data);

    return 0;
}
