#version 410 core

uniform mat4 mvp;

in vec3 position;
in vec3 normal;
in vec2 texcoord;

out vec3 frag_normal;
out vec2 frag_texcoord;

void main()
{
    frag_normal = normal;
    frag_texcoord = texcoord;
    gl_Position = mvp * vec4(position, 1);
}
