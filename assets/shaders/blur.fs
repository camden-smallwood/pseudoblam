#version 410 core

uniform sampler2D blur_texture;

in vec2 frag_texcoord;

out vec4 out_color;

const float weights[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

float gauss(float x, float s2)
{
    float c = 1.0 / (2.0 * 3.14159265359 * s2);
    float e = -(x * x) / (2.0 * s2);
    return c * exp(e);
}

vec4 gaussian_blur(sampler2D image, vec2 uv, vec2 direction)
{
    vec2 resolution = vec2(1280, 720);

    vec4 color = vec4(0.0);

    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;

    color += texture(image, uv) * 0.1964825501511404;
    color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
    color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
    color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
    color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
    color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
    color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
    
    return color;
}

void main()
{
    vec3 color = vec3(0.0);
    
    color += gaussian_blur(blur_texture, frag_texcoord, vec2(1.0, 0.0)).rgb;
    color += gaussian_blur(blur_texture, frag_texcoord, vec2(0.0, 1.0)).rgb;

    out_color = vec4(color, 1.0);
}
