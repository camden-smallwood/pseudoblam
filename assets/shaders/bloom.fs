#version 410 core

uniform sampler2D base_texture;
uniform sampler2D hdr_texture;
uniform bool bloom;
uniform float exposure;

in vec2 frag_texcoord;

out vec4 out_color;

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

    if (bloom)
        hdr_color += bloom_color;

    vec3 color = vec3(1.0) - exp(-hdr_color * exposure);

    // color = calculate_brightness_contrast(color, 0.7, 0.7);
    // color = calculate_gamma(color, 1.0 / 2.2);

    // const float gamma = 2.2;
    // color = pow(color, vec3(1.0 / gamma));

    out_color = vec4(color, 1.0);
}