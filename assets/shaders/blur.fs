#version 410 core

uniform sampler2D blur_texture;
uniform float blur_amount;
uniform bool blur_horizontal;
uniform bool blur_vertical;

in vec2 frag_texcoord;

out vec4 out_color;

float gauss(float x, float s2)
{
    float c = 1.0 / (2.0 * 3.14159265359 * s2);
    float e = -(x * x) / (2.0 * s2);
    return c * exp(e);
}

void main()
{
    float weights[5] = float[](0, 0, 0, 0, 0);
    weights[0] = gauss(0.0, blur_amount);
    float sum = weights[0];

    for (int i = 1; i < 5; i++)
    {
        weights[i] = gauss(i, blur_amount);
        sum += 2.0 * weights[i];
    }

    for (int i = 0; i < 5; i++)
    {
        weights[i] /= sum;
    }

    vec2 texel_size = 1.0 / textureSize(blur_texture, 0);
    vec3 color = texture(blur_texture, frag_texcoord).rgb * weights[0];

    if (blur_horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            color += texture(blur_texture, frag_texcoord + vec2(texel_size.x * i, 0.0)).rgb * weights[i];
            color += texture(blur_texture, frag_texcoord - vec2(texel_size.x * i, 0.0)).rgb * weights[i];
        }
    }

    if (blur_vertical)
    {
        for (int i = 1; i < 5; ++i)
        {
            color += texture(blur_texture, frag_texcoord + vec2(0.0, texel_size.y * i)).rgb * weights[i];
            color += texture(blur_texture, frag_texcoord - vec2(0.0, texel_size.y * i)).rgb * weights[i];
        }
    }

    out_color = vec4(color, 1.0);
}
