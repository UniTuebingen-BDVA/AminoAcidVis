#version 450

uniform sampler2D tex;

out vec4 fragColor;

in vData {
    vec3 vertex;
	vec4 color;
	vec2 uv;
} fragment;

void main() {
	vec4 textureColor = texture(tex, fragment.uv);
    fragColor = fragment.color*(1-textureColor.a) + vec4(textureColor.r,textureColor.g,textureColor.b,1)*textureColor.a; 
}
