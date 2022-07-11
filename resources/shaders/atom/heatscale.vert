#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

uniform mat4 projection;
uniform vec3 colorA;
uniform vec3 colorB;

out vec3 color;

void main() {
    gl_Position = projection*position;
	color = (uv.y > 0.5)? colorB: colorA;// vec3(uv.x,uv.y, colorA.b);//colorA;
}