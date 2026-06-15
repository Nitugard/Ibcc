#version 330

uniform samplerCube skybox;

varying vec3 frag_vertex_uv;

void main() {
    float gamma = 2.2;
    gl_FragColor = pow(textureCube(skybox, frag_vertex_uv), vec4(1.0/gamma));
}