#version 410 core

uniform sampler2D blur_texture;
uniform bool blur_horizontal;

in vec2 frag_texcoord;

out vec3 out_color;

void main()
{
    vec2 blur_texture_size = textureSize(blur_texture, 0).xy;

    vec2 blur_direction = 1.0 / blur_texture_size;
    int blur_size;

    if (blur_horizontal)
    {
        blur_direction.y = 0.0;
        blur_size = int(blur_texture_size.x);
    }
    else
    {
        blur_direction.x = 0.0;
        blur_size = int(blur_texture_size.y);
    }

    vec4 blur_color = vec4(texture(blur_texture, frag_texcoord).rgb, 1.0);

    for (int i = 1; i <= 5; ++ i)
    {
        vec2 blur_offset = float(i) * blur_direction;
        float weight = (1.0 / (2.0 * 3.14157)) * exp(-exp2(float(i) / float(blur_size)) / 2.0);

        blur_color += vec4(texture(blur_texture, frag_texcoord + blur_offset).rgb * weight, weight);
        blur_color += vec4(texture(blur_texture, frag_texcoord - blur_offset).rgb * weight, weight);
    }

    out_color = blur_color.rgb / blur_color.w;
}
