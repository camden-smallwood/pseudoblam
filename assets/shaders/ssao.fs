#version 410 core

uniform sampler2D noise_texture;
uniform sampler2D normal_texture;
uniform sampler2D depth_texture;

in vec2 frag_texcoord;

const float totStrength = 2.38;
const float strength = 0.07;
const float offset = 0.0;
const float falloff = 0.0002;
const float rad = 0.01;
#define SAMPLES 16 // 10 is good
const float invSamples = -2.38 / 16.0;

layout(location = 0) out vec4 out_color;

void main(void)
{
    vec3 pSphere[16] = vec3[](
        vec3(0.53812504, 0.18565957, -0.43192),
        vec3(0.13790712, 0.24864247, 0.44301823),
        vec3(0.33715037, 0.56794053, -0.005789503),
        vec3(-0.6999805, -0.04511441, -0.0019965635),
        vec3(0.06896307, -0.15983082, -0.85477847),
        vec3(0.056099437, 0.006954967, -0.1843352),
        vec3(-0.014653638, 0.14027752, 0.0762037),
        vec3(0.010019933, -0.1924225, -0.034443386),
        vec3(-0.35775623, -0.5301969, -0.43581226),
        vec3(-0.3169221, 0.106360726, 0.015860917),
        vec3(0.010350345, -0.58698344, 0.0046293875),
        vec3(-0.08972908, -0.49408212, 0.3287904),
        vec3(0.7119986, -0.0154690035, -0.09183723),
        vec3(-0.053382345, 0.059675813, -0.5411899),
        vec3(0.035267662, -0.063188605, 0.54602677),
        vec3(-0.47761092, 0.2847911, -0.0271716));
    
    // grab a normal for reflecting the sample rays later on
    vec3 fres = normalize((texture(noise_texture, frag_texcoord * offset).xyz * 2.0) - vec3(1.0));

    vec3 frag_normal = texture(normal_texture, frag_texcoord).xyz;
    float frag_depth = texture(depth_texture, frag_texcoord).r;

    // current fragment coords in screen space
    vec3 ep = vec3(frag_texcoord.xy, frag_depth);

    float bl = 0.0;
    // adjust for the depth ( not shure if this is good..)
    float radD = rad / frag_depth;

    for (int i = 0; i < SAMPLES; i++)
    {
        // get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
        vec3 ray = radD * reflect(pSphere[i], fres);

        // get the depth of the occluder fragment
        vec2 occluder_texcoord = ep.xy + sign(dot(ray, frag_normal)) * ray.xy;
        vec3 occluder_normal = texture(normal_texture, occluder_texcoord).xyz;
        float occluder_depth = texture(depth_texture, occluder_texcoord).r;

        // if occluder_depth_difference is negative = occluder is behind current fragment
        float occluder_depth_difference = frag_depth - occluder_depth;

        // calculate the difference between the normals as a weight
        // the falloff equation, starts at falloff and is kind of 1/x^2 falling
        bl += step(falloff, occluder_depth_difference) * (1.0 - dot(occluder_normal, frag_normal)) * (1.0 - smoothstep(falloff, strength, occluder_depth_difference));
    }

    // output the result
    out_color = vec4(vec3(1.0 + bl * invSamples), 1.0);
}
