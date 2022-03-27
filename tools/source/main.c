#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int tool_execute_compile_shader_command(int argc, const char **argv)
{
}

struct tool_command
{
    const char *name;
    int (*execute)(int argc, const char **argv);
} static tool_commands[] =
{
    {
        "compile-shader",
        tool_execute_compile_shader_command,
    }
};

enum
{
    NUMBER_OF_TOOL_COMMANDS = sizeof(tool_commands) / sizeof(tool_commands[0]),
};

int main(int argc, const char **argv)
{
    if (argc == 0)
    {
        // TODO: print help
        return 0;
    }
    
    for (int i = 0; i < NUMBER_OF_TOOL_COMMANDS; i++)
    {
        struct tool_command *command = tool_commands + i;

        if (strcmp(argv[0], command->name) == 0)
        {
            // TODO: check parameters
            // TODO: call command function
            return 0;
        }
    }

    // TODO: print help

    return 0;
}
