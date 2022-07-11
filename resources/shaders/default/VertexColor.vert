#version 450

layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in vec4 colorAttribute;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vData {
    vec3 vertex;
	vec4 color;
} vertex;


void main(){
	vec4 position = view * model * positionAttribute;
	gl_Position =  projection * view * model * positionAttribute;
	vertex.vertex = vec3(position);
	vertex.color = colorAttribute;
}
