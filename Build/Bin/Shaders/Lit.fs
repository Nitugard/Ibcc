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

uniform sampler2D shadow_map;
uniform mat4      light_space;

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

    float bias    = max(0.005 * (1.0 - NdotL), 0.001);
    float shadow  = 0.0;
    vec2 texel    = 1.0 / vec2(shadow_size);

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closest = texture(shadow_map, proj.xy + vec2(x, y) * texel).r;
            shadow += proj.z - bias > closest ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;
    return shadow * 0.5;
}

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(view_position - position);

    vec3  sun_dir  = normalize(vec3(1.0, 2.0, 1.0));
    vec3  H        = normalize(V + sun_dir);
    vec3  R        = reflect(-V, N);
    float NdotL    = max(dot(N, sun_dir), 0.0);
    float NdotH    = max(dot(N, H),       0.0);

    float r        = clamp(roughness, 0.05, 1.0);
    float m        = clamp(metallic, 0.0, 1.0);
    float shininess = mix(128.0, 4.0, r * r);

    vec3 sun = vec3(1.1, 1.05, 1.0);
    vec3 env = texture(skybox, R).rgb;
    env = max(env, mix(vec3(0.08, 0.09, 0.10), vec3(0.35, 0.45, 0.60), clamp(R.y * 0.5 + 0.5, 0.0, 1.0)));

    vec3 ambient = base_color * 0.18 + vec3(0.04);
    vec3 diffuse = base_color * NdotL * sun * (1.0 - m * 0.8);
    vec3  spec_tint = mix(vec3(0.9), base_color, max(m, 0.25));
    float spec_str  = mix(0.4, 1.0, m);
    vec3  specular  = spec_tint * pow(NdotH, shininess) * NdotL * sun * spec_str;

    float shadow = shadow_calc(NdotL);
    vec3 dielectric = diffuse + specular;
    vec3 metal_reflection = env * base_color * (1.0 - r * 0.7);
    vec3 result = ambient + (1.0 - shadow) * mix(dielectric, metal_reflection + specular, m);

    result = pow(max(result, 0.0), vec3(1.0 / 2.2));
    gl_FragColor = vec4(result, 1.0);
}
