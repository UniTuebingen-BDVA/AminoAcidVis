#version 330

out vec4 fragmentColor;

in vec3 color;

void main() {
	fragmentColor = vec4(color,1);
}