#version 330

/*
 * GGX specular pre-filter for a single roughness level.
 * Run once per mip level (roughness) × 6 cubemap faces at startup.
 *
 * Mip 0 → roughness 0 (mirror).
 * Mip N → roughness 1 (fully diffuse-like specular).
 */

varying vec3 frag_vertex_uv;   /* cube direction, set by Skybox.vs */

uniform samplerCube environment;
uniform float roughness;

const float PI = 3.14159265358979323846;

/* ---- Hammersley low-discrepancy sequence ---- */
float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radical_inverse_vdc(i));
}

/* ---- GGX NDF importance sampling ---- */
vec3 importance_sample_ggx(vec2 Xi, vec3 N, float a)
{
    float phi       = 2.0 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    vec3 H = vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);

    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

void main()
{
    vec3 N = normalize(frag_vertex_uv);
    vec3 V = N;   /* isotropic assumption — V = N avoids the grazing-angle darkening */

    float a = roughness * roughness;

    vec3  total  = vec3(0.0);
    float weight = 0.0;

    const uint SAMPLES = 512u;
    for (uint i = 0u; i < SAMPLES; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLES);
        vec3 H  = importance_sample_ggx(Xi, N, a);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if (NdotL > 0.0) {
            total  += texture(environment, L).rgb * NdotL;
            weight += NdotL;
        }
    }

    gl_FragColor = vec4(total / max(weight, 1e-4), 1.0);
}
