#pragma once
#include <stdio.h>
#include <stdlib.h>

enum glsl_type
{
    _glsl_type_bool,
    _glsl_type_int,
    _glsl_type_uint,
    _glsl_type_float,
    _glsl_type_double,
    _glsl_type_bvec2,
    _glsl_type_bvec3,
    _glsl_type_bvec4,
    _glsl_type_ivec2,
    _glsl_type_ivec3,
    _glsl_type_ivec4,
    _glsl_type_uvec2,
    _glsl_type_uvec3,
    _glsl_type_uvec4,
    _glsl_type_vec2,
    _glsl_type_vec3,
    _glsl_type_vec4,
    _glsl_type_dvec2,
    _glsl_type_dvec3,
    _glsl_type_dvec4,
    _glsl_type_mat2,
    _glsl_type_mat3,
    _glsl_type_mat4,
    _glsl_type_mat2x2,
    _glsl_type_mat2x3,
    _glsl_type_mat2x4,
    _glsl_type_mat3x2,
    _glsl_type_mat3x3,
    _glsl_type_mat3x4,
    _glsl_type_mat4x2,
    _glsl_type_mat4x3,
    _glsl_type_mat4x4,
    _glsl_type_sampler,
    _glsl_type_sampler2D,
};

static inline const char *glsl_type_to_string(enum glsl_type type)
{
    switch (type)
    {
    case _glsl_type_bool:
        return "bool";
    case _glsl_type_int:
        return "int";
    case _glsl_type_uint:
        return "uint";
    case _glsl_type_float:
        return "float";
    case _glsl_type_double:
        return "double";
    case _glsl_type_bvec2:
        return "bvec2";
    case _glsl_type_bvec3:
        return "bvec3";
    case _glsl_type_bvec4:
        return "bvec4";
    case _glsl_type_ivec2:
        return "ivec2";
    case _glsl_type_ivec3:
        return "ivec3";
    case _glsl_type_ivec4:
        return "ivec4";
    case _glsl_type_uvec2:
        return "uvec2";
    case _glsl_type_uvec3:
        return "uvec3";
    case _glsl_type_uvec4:
        return "uvec4";
    case _glsl_type_vec2:
        return "vec2";
    case _glsl_type_vec3:
        return "vec3";
    case _glsl_type_vec4:
        return "vec4";
    case _glsl_type_dvec2:
        return "dvec2";
    case _glsl_type_dvec3:
        return "dvec3";
    case _glsl_type_dvec4:
        return "dvec4";
    case _glsl_type_mat2:
        return "mat2";
    case _glsl_type_mat3:
        return "mat3";
    case _glsl_type_mat4:
        return "mat4";
    case _glsl_type_mat2x2:
        return "mat2x2";
    case _glsl_type_mat2x3:
        return "mat2x3";
    case _glsl_type_mat2x4:
        return "mat2x4";
    case _glsl_type_mat3x2:
        return "mat3x2";
    case _glsl_type_mat3x3:
        return "mat3x3";
    case _glsl_type_mat3x4:
        return "mat3x4";
    case _glsl_type_mat4x2:
        return "mat4x2";
    case _glsl_type_mat4x3:
        return "mat4x3";
    case _glsl_type_mat4x4:
        return "mat4x4";
    case _glsl_type_sampler:
        return "sampler";
    case _glsl_type_sampler2D:
        return "sampler2D";
    default:
        fprintf(stderr, "ERROR: unhandled glsl type %i @ L%i in \"%s\"\n", type, __LINE__, __FILE__);
        exit(EXIT_FAILURE);
    }
}
