#version 410 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool use_nodes;

const int MAXIMUM_NODES = 100;
const int MAXIMUM_NODE_INFLUENCE = 4;
uniform int node_count;
uniform mat4 node_matrices[MAXIMUM_NODES];

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
out vec3 frag_view_position;
out mat3 frag_view_tbn;

void main()
{
    mat4 transform = mat4(1.0);

    if (use_nodes == true)
    {
        transform = mat4(0.0);

        for (int i = 0; i < MAXIMUM_NODE_INFLUENCE; i++)
        {
            int node_index = node_indices[i];

            if (node_index == -1 || node_index >= node_count || node_index >= MAXIMUM_NODES)
                continue;

            transform += node_matrices[node_index] * node_weights[node_index];
        }
    }

    frag_position = vec3(model * transform * vec4(position, 1.0));
    frag_normal = vec3(model * transform * vec4(normal, 0.0));
    frag_texcoord = texcoord;
    
    vec3 T = normalize(vec3(model * transform * vec4(tangent, 0.0)));
    vec3 B = normalize(vec3(model * transform * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * transform * vec4(normal, 0.0)));
    frag_tbn = mat3(T, B, N);

    frag_view_position = vec3(view * model * transform * vec4(position, 1.0));

    mat3 normal_matrix = transpose(inverse(mat3(view * model * transform)));
    T = normalize(normal_matrix * tangent);
    N = normalize(normal_matrix * normal);
    T = normalize(T - dot(T, N) * N);
    B = cross(N, T);
    frag_view_tbn = mat3(T, B, N);

    gl_Position = projection * view * model * transform * vec4(position, 1.0);
}
