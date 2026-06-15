#version 330 core

in vec3 vertex_pos;
in vec3 vertex_normal;
in vec4 vertex_tangent;
in vec4 vertex_color;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

out vec3 frag_position;
out vec3 frag_normal;
out vec4 frag_tangent;
out vec4 frag_color;

void main() {
    frag_position = (model * vec4(vertex_pos, 1.0)).xyz;
    frag_normal = mat3(transpose(inverse(model))) * vertex_normal;
    frag_tangent = vertex_tangent;
    frag_color = vertex_color;
    gl_Position = projection * view * vec4(frag_position, 1.0);
}
