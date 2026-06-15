#version 330 core

uniform samplerCube skybox;

in vec3 frag_vertex_uv;

out vec4 out_color;

void main() {
    out_color = pow(texture(skybox, frag_vertex_uv), vec4(1.0 / 2.2));
}
