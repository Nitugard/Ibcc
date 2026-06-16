#version 330

varying vec3 position;
varying vec3 normal;
varying vec4 tangent;
varying vec4 color;

uniform vec3  view_position;
uniform samplerCube skybox;

uniform vec3  base_color;
uniform float roughness;
uniform float metallic;
uniform float exposure;

uniform sampler2D shadow_map;
uniform mat4      light_space;
uniform sampler2D brdf_lut;

/* ---- PCF shadow ---- */
float shadow_calc(float NdotL)
{
    vec4 ls   = light_space * vec4(position, 1.0);
    vec3 proj = ls.xyz / ls.w * 0.5 + 0.5;
    if (proj.x < 0.0 || proj.x > 1.0 ||
        proj.y < 0.0 || proj.y > 1.0 ||
        proj.z < 0.0 || proj.z > 1.0) {
        return 0.0;
    }

    ivec2 shadow_size = textureSize(shadow_map, 0);
    if (shadow_size.x <= 0 || shadow_size.y <= 0) {
        return 0.0;
    }

    float bias   = max(0.005 * (1.0 - NdotL), 0.001);
    float shadow = 0.0;
    vec2 texel   = 1.0 / vec2(shadow_size);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closest = texture(shadow_map, proj.xy + vec2(x, y) * texel).r;
            shadow += proj.z - bias > closest ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    return shadow * 0.5;
}

/* ---- Schlick Fresnel ---- */
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

/* ---- Environment sample with horizon fallback ---- */
vec3 env_sample(vec3 dir)
{
    vec3 s = texture(skybox, dir).rgb;
    vec3 floor_color = mix(vec3(0.08, 0.09, 0.10),
                           vec3(0.35, 0.45, 0.60),
                           clamp(dir.y * 0.5 + 0.5, 0.0, 1.0));
    return max(s, floor_color);
}

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(view_position - position);
    vec3 R = reflect(-V, N);

    vec3  sun_dir = normalize(vec3(1.0, 2.0, 1.0));
    vec3  H       = normalize(V + sun_dir);
    float NdotL   = max(dot(N, sun_dir), 0.0);
    float NdotH   = max(dot(N, H),       0.0);
    float NdotV   = max(dot(N, V),       0.0);

    float r = clamp(roughness, 0.05, 1.0);
    float m = clamp(metallic,  0.0,  1.0);

    /*
     * PBR F0:
     *   dielectrics → 0.04 (common non-metal reflectance)
     *   metals      → base_color (metals tint their reflections)
     */
    vec3 F0 = mix(vec3(0.04), base_color, m);

    /* Fresnel at view angle (drives IBL split) and at half-vector (direct spec) */
    vec3 F_ibl    = fresnel_schlick(NdotV, F0);
    vec3 F_direct = fresnel_schlick(NdotH, F0);

    /*
     * Diffuse fraction: energy not taken by specular, and zero for metals.
     * (1 - F) gives conservation; * (1 - m) kills diffuse for metals.
     */
    vec3 kD = (1.0 - F_ibl) * (1.0 - m);

    /* ---- IBL — split-sum approximation ---- */

    /*
     * Specular: pre-filtered environment (skybox ≈ fully sharp, so we dim
     * by roughness to fake the blurring a real pre-filter would give).
     */
    vec3 env_spec = env_sample(R) * (1.0 - r * 0.75);

    /*
     * Diffuse irradiance: sample skybox along N as an upper-hemisphere
     * average (no separate irradiance map needed).
     */
    vec3 irradiance = env_sample(N) * 0.45;

    /*
     * BRDF LUT lookup: (NdotV, roughness) → (scale, bias).
     *   specular_ibl = prefilteredColor * (F0 * scale + bias)
     * This correctly accounts for Fresnel and geometry shadowing
     * across all view angles and roughness values.
     */
    vec2 brdf      = texture(brdf_lut, vec2(NdotV, r)).rg;
    vec3 spec_ibl  = env_spec * (F0 * brdf.x + brdf.y);

    vec3 ambient = kD * base_color * irradiance + spec_ibl;

    /* ---- Direct lighting ---- */
    vec3 sun = vec3(1.1, 1.05, 1.0);

    float shininess = mix(256.0, 4.0, r * r);
    vec3  diffuse   = kD * base_color * NdotL * sun;
    vec3  specular  = F_direct * pow(NdotH, shininess) * NdotL * sun;

    float shadow = shadow_calc(NdotL);
    vec3  result = ambient + (1.0 - shadow) * (diffuse + specular);

    /* HDR tonemap + gamma */
    result = vec3(1.0) - exp(-max(result, 0.0) * exposure);
    result = pow(result, vec3(1.0 / 2.2));
    gl_FragColor = vec4(result, 1.0);
}
