#version 330
attribute vec3 vertex_pos;
attribute vec3 vertex_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float outline_scale;
void main() {
    vec3 expanded = vertex_pos + normalize(vertex_normal) * outline_scale;
    gl_Position = projection * view * model * vec4(expanded, 1.0);
}
