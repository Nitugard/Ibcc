#version 330

/*
 * BRDF integration LUT — split-sum approximation (Epic Games 2013).
 *
 * Output (stored as RG):
 *   R = scale : integral of f_r(l,v) * NdotL * (1 - (1-VdotH)^5) dω
 *   G = bias  : integral of f_r(l,v) * NdotL *      (1-VdotH)^5  dω
 *
 * UV axes: X = NdotV (0→1), Y = roughness (0→1)
 *
 * Usage in Lit.fs:
 *   vec2 brdf = texture(brdf_lut, vec2(NdotV, roughness)).rg;
 *   vec3 spec_ibl = prefiltered_env * (F0 * brdf.x + brdf.y);
 */

varying vec2 uv;

const float PI = 3.14159265358979323846;

/* ---- Van der Corput radical inverse (base 2) ---- */
float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; /* / 0x100000000 */
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radical_inverse_vdc(i));
}

/* ---- GGX NDF importance sampling ---- */
vec3 importance_sample_ggx(vec2 Xi, vec3 N, float roughness)
{
    float a         = roughness * roughness;
    float phi       = 2.0 * PI * Xi.x;
    float cos_theta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    /* Tangent-space half-vector */
    vec3 H = vec3(cos(phi) * sin_theta,
                  sin(phi) * sin_theta,
                  cos_theta);

    /* Build TBN from N */
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

/* ---- Smith masking-shadowing (IBL form: k = a^2/2) ---- */
float geometry_schlick_ggx(float NdotX, float roughness)
{
    float a = roughness;
    float k = (a * a) * 0.5;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float geometry_smith(float NdotV, float NdotL, float roughness)
{
    return geometry_schlick_ggx(NdotV, roughness)
         * geometry_schlick_ggx(NdotL, roughness);
}

void main()
{
    float NdotV     = max(uv.x, 1e-4);   /* avoid divide-by-zero at NdotV=0 */
    float roughness = max(uv.y, 0.05);   /* clamp to match Lit.fs clamp      */

    /*
     * Working in the canonical frame where N = (0,0,1).
     * V lies in the X-Z plane: V = (sin θ, 0, cos θ) where cos θ = NdotV.
     */
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);

    float A = 0.0;  /* Fresnel scale */
    float B = 0.0;  /* Fresnel bias  */

    const uint SAMPLES = 1024u;
    for (uint i = 0u; i < SAMPLES; ++i)
    {
        vec2 Xi = hammersley(i, SAMPLES);
        vec3 H  = importance_sample_ggx(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0)
        {
            float G     = geometry_smith(NdotV, NdotL, roughness);
            float G_vis = (G * VdotH) / max(NdotH * NdotV, 1e-4);
            float Fc    = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * G_vis;
            B +=        Fc  * G_vis;
        }
    }

    A /= float(SAMPLES);
    B /= float(SAMPLES);

    gl_FragColor = vec4(A, B, 0.0, 1.0);
}
