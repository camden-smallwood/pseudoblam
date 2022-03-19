#version 410 core

uniform sampler2D blur_texture;
uniform bool blur_horizontal;

in vec2 frag_texcoord;

out vec4 out_color;

void main()
{
    vec2 blur_direction = 1.0 / textureSize(blur_texture, 0);
    vec2 blur_texcoord = frag_texcoord;
    vec3 blur_color = vec3(0);

    if (blur_horizontal)
        blur_direction.y = 0;
    else
        blur_direction.x = 0;

    blur_texcoord -= 5.0 * blur_direction;		// -5
    blur_color += (1/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// -4
    blur_color += (10/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// -3
    blur_color += (45/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// -2
    blur_color += (120/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// -1
    blur_color += (210/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// 0
    blur_color += (252/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// +1
    blur_color += (210/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// +2
    blur_color += (120/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// +3
    blur_color += (45/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// +4
    blur_color += (10/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    blur_texcoord += blur_direction;			// +5
    blur_color += (1/1024.0) * texture(blur_texture, blur_texcoord).rgb;

    out_color = vec4(blur_color, 1.0);
}
