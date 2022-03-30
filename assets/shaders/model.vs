#version 410 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#define MAXIMUM_NUMBER_OF_MODEL_NODES 253
#define MAXIMUM_NODE_INFLUENCE 4

uniform mat4 node_matrices[MAXIMUM_NUMBER_OF_MODEL_NODES];

in vec3 position;
in vec3 normal;
in vec2 texcoord;
in vec3 tangent;
in vec3 bitangent;
in ivec4 node_indices;
in vec4 node_weights;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;
out mat3 frag_tbn;

void main()
{
    mat4 transform = mat4(0.0);

    transform += node_matrices[node_indices[0]] * node_weights[0];
    transform += node_matrices[node_indices[1]] * node_weights[1];
    transform += node_matrices[node_indices[2]] * node_weights[2];
    transform += node_matrices[node_indices[3]] * node_weights[3];
    
    if (transform == mat4(0.0))
        transform = mat4(1.0);

    frag_position = vec3(model * transform * vec4(position, 1.0));
    frag_normal = normal;
    frag_texcoord = texcoord;
    
    mat3 normal_matrix = transpose(inverse(mat3(model * transform)));

    frag_tbn = mat3(
        normalize(normal_matrix * tangent),
        normalize(normal_matrix * bitangent),
        normalize(normal_matrix * normal));

    gl_Position = projection * view * model * transform * vec4(position, 1.0);
}
