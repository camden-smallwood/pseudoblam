/*
COMMANDS.H
    Tool command management and declarations.
*/

#pragma once
#include <stddef.h>

/* ---------- constants */

enum command_parameter_type
{
    _command_parameter_string,
    _command_parameter_float,
    _command_parameter_int,
};

enum command_parameter_flags
{
    _command_parameter_optional_bit,
};

/* ---------- structures */

struct command_parameter_definition
{
    const char *name;
    enum command_parameter_type type;
    unsigned int flags;
};

struct command_definition
{
    const char *name;
    const char *description;
    size_t parameter_count;
    const struct command_parameter_definition *parameters;
    int(*execute)(int argc, const char **argv);
};

/* ---------- prototypes/COMMANDS.C */

const struct command_definition *command_find_definition(const char *name);
