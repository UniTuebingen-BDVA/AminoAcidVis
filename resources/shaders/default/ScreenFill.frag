#version 330

in vec2 passTCoord;

uniform sampler2D diffusetexture;

out vec4 fragmentColor;

void main() {
    fragmentColor = texture(diffusetexture, passTCoord);
}