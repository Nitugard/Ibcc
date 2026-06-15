#version 330

varying vec3 color;

void main() {
    float gamma = 2.2;
    gl_FragColor = pow(vec4(color, 1.0), vec4(1.0/gamma));
}