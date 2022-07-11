#version 450

out vec4 fragColor;

in vData {
    vec3 vertex;
	vec4 color;
} fragment;

void main() {
    fragColor = fragment.color;
}
