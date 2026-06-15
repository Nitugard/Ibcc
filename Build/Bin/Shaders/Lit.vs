#version 330

attribute vec3 vertex_pos;
attribute vec3 vertex_normal;
attribute vec4 vertex_tangent;
attribute vec4 vertex_color;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

varying vec3 position;
varying vec3 normal;
varying vec4 tangent;
varying vec4 color;

void main() {
    position = (model * vec4(vertex_pos, 1.0)).xyz;
    normal = mat3(transpose(inverse(model))) * vertex_normal;
    tangent = vertex_tangent;
    color = vertex_color;
    gl_Position = projection * view * vec4(position, 1.0);


}