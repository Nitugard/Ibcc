#version 330

varying vec3 position;
varying vec3 normal;
varying vec4 tangent;
varying vec4 color;
varying vec2 uv;

uniform vec3  view_position;
uniform samplerCube skybox;
uniform samplerCube prefiltered_env;

uniform vec3  base_color;
uniform float roughness;
uniform float metallic;
uniform float exposure;

uniform sampler2D shadow_map;
uniform mat4      light_space;
uniform sampler2D brdf_lut;

uniform sampler2D base_color_texture;
uniform int       has_color_texture;
uniform int       has_vertex_color;

#define PREFILTER_MIPS 5

/* ---- ACES filmic tone mapping (Narkowicz 2015) ---- */
vec3 aces_filmic(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

/* ---- Poisson disk 16-sample PCF soft shadows ---- */
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

    vec2 poisson_disk[16];
    poisson_disk[ 0] = vec2(-0.94201624, -0.39906216);
    poisson_disk[ 1] = vec2( 0.94558609, -0.76890725);
    poisson_disk[ 2] = vec2(-0.09418410, -0.92938870);
    poisson_disk[ 3] = vec2( 0.34495938,  0.29387760);
    poisson_disk[ 4] = vec2(-0.91588581,  0.45771432);
    poisson_disk[ 5] = vec2(-0.81544232, -0.87912464);
    poisson_disk[ 6] = vec2(-0.38277543,  0.27676845);
    poisson_disk[ 7] = vec2( 0.97484398,  0.75648379);
    poisson_disk[ 8] = vec2( 0.44323325, -0.97511554);
    poisson_disk[ 9] = vec2( 0.53742981, -0.47373420);
    poisson_disk[10] = vec2(-0.26496911, -0.41893023);
    poisson_disk[11] = vec2( 0.79197514,  0.19090188);
    poisson_disk[12] = vec2(-0.24188840,  0.99706507);
    poisson_disk[13] = vec2(-0.81409955,  0.91437590);
    poisson_disk[14] = vec2( 0.19984126,  0.78641367);
    poisson_disk[15] = vec2( 0.14383161, -0.14100790);

    float theta = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453)
                  * 6.28318;
    mat2 rot = mat2(cos(theta), -sin(theta), sin(theta), cos(theta));

    float bias   = max(0.003 * (1.0 - NdotL), 0.0008);
    float shadow = 0.0;
    vec2  texel  = 1.0 / vec2(shadow_size);
    const float SOFTNESS = 2.5;

    for (int i = 0; i < 16; ++i) {
        vec2  offset  = rot * poisson_disk[i] * texel * SOFTNESS;
        float closest = texture(shadow_map, proj.xy + offset).r;
        shadow += proj.z - bias > closest ? 1.0 : 0.0;
    }

    shadow /= 16.0;
    return shadow * 0.75;
}

/* ---- Schlick Fresnel ---- */
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

/* ---- Diffuse irradiance from raw skybox ---- */
vec3 env_irradiance(vec3 dir)
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

    /* Resolve albedo: base_color_factor × texture × vertex_color (GLTF spec) */
    vec3 albedo = base_color;
    if (has_color_texture != 0)
        albedo *= texture(base_color_texture, uv).rgb;
    if (has_vertex_color != 0)
        albedo *= color.rgb;

    vec3 F0 = mix(vec3(0.04), albedo, m);

    vec3 F_ibl    = fresnel_schlick(NdotV, F0);
    vec3 F_direct = fresnel_schlick(NdotH, F0);

    vec3 kD = (1.0 - F_ibl) * (1.0 - m);

    /* Specular IBL: pre-filtered env at roughness mip */
    float mip_level = r * float(PREFILTER_MIPS - 1);
    vec3  env_spec  = textureLod(prefiltered_env, R, mip_level).rgb;

    /* Diffuse IBL: raw skybox along N */
    vec3 irradiance = env_irradiance(N) * 0.45;

    /* BRDF LUT split-sum */
    vec2 brdf     = texture(brdf_lut, vec2(NdotV, r)).rg;
    vec3 spec_ibl = env_spec * (F0 * brdf.x + brdf.y);

    vec3 ambient = kD * albedo * irradiance + spec_ibl;

    /* Direct lighting */
    vec3 sun = vec3(1.1, 1.05, 1.0);

    float shininess = mix(256.0, 4.0, r * r);
    vec3  diffuse   = kD * albedo * NdotL * sun;
    vec3  specular  = F_direct * pow(NdotH, shininess) * NdotL * sun;

    float shadow = shadow_calc(NdotL);
    vec3  result = ambient + (1.0 - shadow) * (diffuse + specular);

    /* Exposure → ACES filmic tonemap → gamma */
    result = aces_filmic(result * exposure);
    result = pow(result, vec3(1.0 / 2.2));
    gl_FragColor = vec4(result, 1.0);
}
