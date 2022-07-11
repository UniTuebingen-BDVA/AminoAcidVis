#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

uniform mat4 view;
uniform mat4 projection;

out vec2 passTCoord;
flat out mat4 passInvViewProj;

void main() {
    gl_Position = position;
    passTCoord = uv;
	passInvViewProj = inverse(projection*view);
}