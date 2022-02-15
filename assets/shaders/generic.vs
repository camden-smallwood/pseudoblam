#version 410 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#define MAXIMUM_BONES 256
uniform uint bone_count;
uniform mat4 bone_matrices[MAXIMUM_BONES];

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec3 tangent;
in vec3 bitangent;
in ivec4 bone_indices;
in vec4 bone_weights;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;
out mat3 frag_tbn;

void main()
{
    frag_position = vec3(model * vec4(position, 1.0));
    frag_normal = vec3(model * vec4(normal, 1.0));
    frag_texcoord = texcoord;
    
    vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,    0.0)));
    frag_tbn = mat3(T, B, N);

    gl_Position = projection * view * model * vec4(position, 1);
}
