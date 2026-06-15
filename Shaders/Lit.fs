#version 330 core

in vec3 frag_position;
in vec3 frag_normal;
in vec4 frag_tangent;
in vec4 frag_color;

uniform vec3 view_position;
uniform samplerCube skybox;
uniform vec3 base_color;
uniform float roughness;
uniform float metallic;

out vec4 out_color;

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 32u;

float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint n)
{
    return vec2(float(i) / float(n), radical_inverse_vdc(i));
}

vec3 importance_sample_ggx(vec2 xi, vec3 n, float material_roughness)
{
    float a = material_roughness * material_roughness;
    float phi = 2.0 * PI * xi.x;
    float cos_theta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    vec3 h;
    h.x = cos(phi) * sin_theta;
    h.y = sin(phi) * sin_theta;
    h.z = cos_theta;

    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);
    return normalize(tangent * h.x + bitangent * h.y + n * h.z);
}

vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 n = normalize(frag_normal);
    vec3 v = normalize(view_position - frag_position);
    vec3 r = reflect(-v, n);
    vec3 albedo = clamp(base_color, vec3(0.0), vec3(1.0));
    float material_roughness = clamp(roughness, 0.04, 1.0);
    float material_metallic = clamp(metallic, 0.0, 1.0);

    float total_weight = 0.0;
    vec3 specular_radiance = vec3(0.0);
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 xi = hammersley(i, SAMPLE_COUNT);
        vec3 h = importance_sample_ggx(xi, n, material_roughness);
        vec3 l = normalize(2.0 * dot(v, h) * h - v);

        float ndotl = max(dot(n, l), 0.0);
        specular_radiance += texture(skybox, l).rgb * ndotl;
        total_weight += ndotl;
    }

    specular_radiance = total_weight > 0.0 ? specular_radiance / total_weight : texture(skybox, r).rgb;
    vec3 diffuse_radiance = texture(skybox, n).rgb;
    vec3 f0 = mix(vec3(0.04), albedo, material_metallic);
    vec3 fresnel = fresnel_schlick(max(dot(n, v), 0.0), f0);
    vec3 diffuse = diffuse_radiance * albedo * (1.0 - material_metallic);
    vec3 specular = specular_radiance * fresnel;
    vec3 color = diffuse * (1.0 - fresnel) + specular;

    out_color = vec4(pow(max(color, vec3(0.0)), vec3(1.0 / 2.2)), 1.0);
}
