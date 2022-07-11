#version 330

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 uv;

out vec2 passTCoord;

void main() {
    gl_Position = position;
    passTCoord = uv;
}