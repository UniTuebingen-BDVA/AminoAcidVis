#version 330

in vec3 tra_color;

out vec4 fragColor;

void main() {
    fragColor = vec4(tra_color,1);
}
