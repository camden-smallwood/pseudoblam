#version 410 core

uniform sampler2D base_texture;
uniform sampler2D hdr_texture;
uniform bool bloom;
uniform float exposure;

in vec2 frag_texcoord;

layout(location = 0) out vec3 out_color;

vec3 calculate_brightness_contrast(vec3 color, float brightness, float contrast)
{
    return ((color - 0.5) * (contrast + 0.5)) + brightness;
}

vec3 calculate_gamma(vec3 value, float param)
{
    return vec3(pow(abs(value.r), param), pow(abs(value.g), param), pow(abs(value.b), param));
}

void main()
{
    vec3 hdr_color = texture(base_texture, frag_texcoord).rgb;
    vec3 bloom_color = texture(hdr_texture, frag_texcoord).rgb;

    out_color = hdr_color + bloom_color;
}
