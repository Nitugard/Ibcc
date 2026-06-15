#version 330 core

in vec3 frag_color;

out vec4 out_color;

void main() {
    out_color = pow(vec4(frag_color, 1.0), vec4(1.0 / 2.2));
}
