#version 410 core

uniform sampler2D position_texture;
uniform sampler2D normal_texture;
uniform sampler2D noise_texture;

uniform uint screen_width;
uniform uint screen_height;

uniform mat4 projection;

uniform float kernel_radius;
uniform float kernel_bias;

#define NUMBER_OF_SSAO_KERNEL_SAMPLES 32
uniform uint kernel_sample_count;
uniform vec3 kernel_samples[NUMBER_OF_SSAO_KERNEL_SAMPLES];

#define SSAO_NOISE_TEXTURE_WIDTH 4
#define SSAO_NOISE_TEXTURE_HEIGHT 4
#define NUMBER_OF_SSAO_NOISE_POINTS SSAO_NOISE_TEXTURE_WIDTH * SSAO_NOISE_TEXTURE_HEIGHT

in vec2 frag_texcoord;

layout(location = 0) out vec4 out_color;

void main()
{
    vec2 noise_scale = vec2(screen_width / SSAO_NOISE_TEXTURE_WIDTH, screen_height / SSAO_NOISE_TEXTURE_HEIGHT);
    
    vec3 frag_position = texture(position_texture, frag_texcoord).xyz;
    vec3 normal = texture(normal_texture, frag_texcoord).rgb;
    vec3 randomVec = texture(noise_texture, frag_texcoord * noise_scale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    for (int i = 0; i < kernel_sample_count; ++i)
    {
        vec3 samplePos = TBN * kernel_samples[i]; // from tangent to view-space
        samplePos = frag_position + samplePos * kernel_radius; 

        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0 

        float sampleDepth = texture(position_texture, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, kernel_radius / abs(frag_position.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + kernel_bias ? 1.0 : 0.0) * rangeCheck;
    }

    out_color = vec4(vec3(1.0 - (occlusion / kernel_sample_count)), 1.0);
}
