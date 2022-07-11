#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 tra_color;

void main()
{
	gl_Position = projection * view * model * vec4(position,1);
	tra_color = color;
}