#version 410 core

struct material_data
{
    vec3 diffuse_color;
    sampler2D diffuse_texture;

    sampler2D normal_texture;
    float bump_scaling;

    vec3 ambient_color;
    float ambient_amount;

    sampler2D specular_texture;
    float specular_amount;
    float specular_shininess;

    sampler2D emissive_texture;
};
uniform material_data material;

in vec3 frag_position;
in vec3 frag_normal;
in vec2 frag_texcoord;
in mat3 frag_tbn;
in mat3 frag_view_tbn;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_albedo_specular;
layout(location = 3) out vec4 out_material;
layout(location = 4) out vec3 out_emissive;
layout(location = 5) out vec3 out_view_normal;

void main()
{
    out_position = frag_position;
    
    vec3 normal_map = (texture(material.normal_texture, frag_texcoord).rgb * 2.0 - 1.0) * vec3(1.0, 1.0, 1.0 / material.bump_scaling);
    out_normal = normalize(frag_tbn * normal_map);

    out_albedo_specular.rgb = texture(material.diffuse_texture, frag_texcoord).rgb;
    out_albedo_specular.a = texture(material.specular_texture, frag_texcoord).r;
    out_material = vec4(material.ambient_amount, material.specular_amount, material.specular_shininess, 0);
    out_emissive = texture(material.emissive_texture, frag_texcoord).rgb;
    
    out_view_normal = normalize(frag_view_tbn * frag_normal) + normalize(frag_view_tbn * normal_map);
}
