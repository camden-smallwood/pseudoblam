#version 410 core

uniform sampler2D noise_texture;
uniform sampler2D normal_texture;
uniform sampler2D depth_texture;

#define NUMBER_OF_SSAO_KERNEL_SAMPLES 64
uniform vec3 kernel_samples[NUMBER_OF_SSAO_KERNEL_SAMPLES];

uniform float strength = 0.025;
uniform float falloff = 0.00005;
uniform float radius = 0.1;

in vec2 frag_texcoord;

layout(location = 0) out float out_color;

void main(void)
{
    vec3 frag_normal = texture(normal_texture, frag_texcoord).xyz;
    float frag_depth = texture(depth_texture, frag_texcoord).r;

    vec2 noise_scale = textureSize(normal_texture, 0).xy / textureSize(noise_texture, 0).xy;
    vec3 noise_sample = normalize((texture(noise_texture, frag_texcoord * noise_scale).xyz * 2.0) - vec3(1.0));

    float occlusion = 0.0;

    for (int i = 0; i < NUMBER_OF_SSAO_KERNEL_SAMPLES; i++)
    {
        // get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
        vec3 ray = radius * reflect(kernel_samples[i], noise_sample);

        // get the depth of the occluder fragment
        vec2 occluder_texcoord = frag_texcoord + sign(dot(ray, frag_normal)) * ray.xy;
        vec3 occluder_normal = texture(normal_texture, occluder_texcoord).xyz;
        float occluder_depth = texture(depth_texture, occluder_texcoord).r;

        // if occluder_depth_difference is negative = occluder is behind current fragment
        float occluder_depth_difference = frag_depth - occluder_depth;

        // calculate the difference between the normals as a weight
        // the falloff equation, starts at falloff and is kind of 1/x^2 falling
        occlusion += step(falloff, occluder_depth_difference)
            * (1.0 - dot(occluder_normal, frag_normal))
            * (1.0 - smoothstep(falloff, strength, occluder_depth_difference));
    }

    out_color = 1.0 - (occlusion / float(NUMBER_OF_SSAO_KERNEL_SAMPLES));
}
