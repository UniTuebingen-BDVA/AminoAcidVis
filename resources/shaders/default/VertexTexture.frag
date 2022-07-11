#version 450

uniform sampler2D tex;

out vec4 fragColor;

in vData {
    vec3 vertex;
	vec2 uv;
} fragment;

void main() {
    fragColor = texture(tex, fragment.uv);
}
