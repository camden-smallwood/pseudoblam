#version 410 core

const float blur_weights[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

uniform sampler2D blur_texture;
uniform bool blur_horizontal;

in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec2 pixel_size = 1.0 / textureSize(blur_texture, 0);
    vec2 texcoord = frag_texcoord;
    vec3 color = texture(blur_texture, texcoord).rgb * blur_weights[0];

    if (blur_horizontal)
    {
        // for (int i = 1; i < 5; ++i)
        // {
        //     color += texture(blur_texture, texcoord + vec2(pixel_size.x * i, 0.0)).rgb * blur_weights[i];
        //     color += texture(blur_texture, texcoord - vec2(pixel_size.x * i, 0.0)).rgb * blur_weights[i];
        // }

        texcoord.x -= 5.0 * pixel_size.x;		// -5
        color += (1/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// -4
        color += (10/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// -3
        color += (45/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// -2
        color += (120/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// -1
        color += (210/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// 0
        color += (252/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// +1
        color += (210/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// +2
        color += (120/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// +3
        color += (45/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// +4
        color += (10/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.x += pixel_size.x;			// +5
        color += (1/1024.0) * texture(blur_texture, texcoord).rgb;
    }
    else
    {
        texcoord.y -= 5.0 * pixel_size.y;		// -5
        color += (1/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// -4
        color += (10/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// -3
        color += (45/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// -2
        color += (120/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// -1
        color += (210/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// 0
        color += (252/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// +1
        color += (210/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// +2
        color += (120/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// +3
        color += (45/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// +4
        color += (10/1024.0) * texture(blur_texture, texcoord).rgb;

        texcoord.y += pixel_size.y;			// +5
        color += (1/1024.0) * texture(blur_texture, texcoord).rgb;
    }

    out_color = vec4(color, 1.0);
}
