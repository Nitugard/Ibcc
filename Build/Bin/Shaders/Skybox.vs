#version 330

attribute vec3 vertex_pos;

uniform mat4 projection;
uniform mat4 view;

varying vec3 frag_vertex_uv;

void main() {
    vec4 v = projection * view * vec4(vertex_pos, 1.0);
    gl_Position = v.xyww;

    frag_vertex_uv = vertex_pos;
}