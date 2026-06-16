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
    if (proj.z > 1.0) return 0.0;
    float closest = texture2D(shadow_map, proj.xy).r;
    float bias    = max(0.005 * (1.0 - NdotL), 0.001);
    return proj.z - bias > closest ? 0.5 : 0.0;
}

void main()
{
    vec3 N = normalize(normal);
    vec3 V = normalize(view_position - position);

    vec3  sun_dir = normalize(vec3(1.0, 2.0, 1.0));
    vec3  H       = normalize(V + sun_dir);
    float NdotL   = max(dot(N, sun_dir), 0.0);
    float NdotH   = max(dot(N, H),       0.0);

    float r        = clamp(roughness, 0.05, 1.0);
    float shininess = mix(128.0, 4.0, r * r);

    vec3 sun = vec3(1.1, 1.05, 1.0);

    vec3 ambient = base_color * 0.18 + vec3(0.04);
    vec3 diffuse = base_color * NdotL * sun * (1.0 - metallic * 0.8);
    vec3  spec_tint = mix(vec3(0.9), base_color, max(metallic, 0.25));
    float spec_str  = mix(0.4, 1.0, metallic);
    vec3  specular  = spec_tint * pow(NdotH, shininess) * NdotL * sun * spec_str;

    float shadow = shadow_calc(NdotL);
    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    result = pow(max(result, 0.0), vec3(1.0 / 2.2));
    gl_FragColor = vec4(result, 1.0);
}
