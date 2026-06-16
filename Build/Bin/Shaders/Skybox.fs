#version 330

uniform samplerCube skybox;

varying vec3 frag_vertex_uv;

void main() {
    gl_FragColor = textureCube(skybox, frag_vertex_uv);
}
