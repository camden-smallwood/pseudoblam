#version 410 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec3 frag_position;
out vec3 frag_normal;
out vec2 frag_texcoord;

void main()
{
    frag_position = vec3(model * vec4(position, 1.0));
    frag_normal = vec3(model * vec4(normal, 1.0));
    frag_texcoord = texcoord * 2;
    gl_Position = projection * view * model * vec4(position, 1);
}
