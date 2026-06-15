#version 330 core

in vec3 vertex_pos;

uniform mat4 projection;
uniform mat4 view;

out vec3 frag_vertex_uv;

void main() {
    vec4 position = projection * view * vec4(vertex_pos, 1.0);
    gl_Position = position.xyww;
    frag_vertex_uv = vertex_pos;
}
