#version 330

attribute vec3 vertex_pos;
attribute vec3 vertex_color;

uniform mat4 projection;
uniform mat4 view;

varying vec3 color;

void main() {
    gl_Position = projection * view * vec4(vertex_pos, 1.0);
    color = vertex_color;
}