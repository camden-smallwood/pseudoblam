#version 410 core

uniform sampler2D quad_texture;

in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    out_color = texture(quad_texture, frag_texcoord);
}
