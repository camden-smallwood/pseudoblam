#version 410 core

out vec4 out_color;

in vec2 frag_texcoord;

uniform sampler2D blur_texture;

uniform bool blur_horizontal;
uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(blur_texture, 0); // gets size of single texel
    vec3 result = texture(blur_texture, frag_texcoord).rgb * weight[0];

    if (blur_horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(blur_texture, frag_texcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(blur_texture, frag_texcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(blur_texture, frag_texcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(blur_texture, frag_texcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    out_color = vec4(result, 1.0);
}
