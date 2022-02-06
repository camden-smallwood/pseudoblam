#version 410 core

uniform sampler2D diffuse_texture;

in vec3 frag_normal;
in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    out_color = texture(diffuse_texture, frag_texcoord);
}
