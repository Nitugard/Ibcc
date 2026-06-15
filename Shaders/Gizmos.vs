#version 330 core

in vec3 vertex_pos;
in vec3 vertex_color;

uniform mat4 projection;
uniform mat4 view;

out vec3 frag_color;

void main() {
    gl_Position = projection * view * vec4(vertex_pos, 1.0);
    frag_color = vertex_color;
}
