#version 410 core

in vec2 position;
in vec2 texcoord;

out vec2 frag_texcoord;

void main()
{
    frag_texcoord = texcoord;
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
}
