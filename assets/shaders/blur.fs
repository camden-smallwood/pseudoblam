#version 410 core

const float blur_weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

uniform sampler2D blur_texture;
uniform bool blur_horizontal;

in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec2 offset = 1.0 / textureSize(blur_texture, 0);
    vec3 result = texture(blur_texture, frag_texcoord).rgb * blur_weights[0];

    if (blur_horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(blur_texture, frag_texcoord + vec2(offset.x * i, 0.0)).rgb * blur_weights[i];
            result += texture(blur_texture, frag_texcoord - vec2(offset.x * i, 0.0)).rgb * blur_weights[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(blur_texture, frag_texcoord + vec2(0.0, offset.y * i)).rgb * blur_weights[i];
            result += texture(blur_texture, frag_texcoord - vec2(0.0, offset.y * i)).rgb * blur_weights[i];
        }
    }

    out_color = vec4(result, 1.0);
}
