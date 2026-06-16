#version 330
attribute vec3 vertex_pos;
uniform mat4 model;
uniform mat4 light_space;
void main() {
    gl_Position = light_space * model * vec4(vertex_pos, 1.0);
}
