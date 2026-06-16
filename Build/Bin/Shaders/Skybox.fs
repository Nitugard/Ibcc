#version 330

uniform samplerCube skybox;
uniform float exposure;

varying vec3 frag_vertex_uv;

vec3 aces_filmic(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    vec3 dir = normalize(frag_vertex_uv);
    vec3 env = texture(skybox, dir).rgb;
    vec3 fallback = mix(vec3(0.08, 0.09, 0.10),
                        vec3(0.35, 0.45, 0.60),
                        clamp(dir.y * 0.5 + 0.5, 0.0, 1.0));
    env = max(env, fallback);

    /* Exposure → ACES filmic → gamma */
    vec3 mapped = aces_filmic(env * exposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));
    gl_FragColor = vec4(mapped, 1.0);
}
