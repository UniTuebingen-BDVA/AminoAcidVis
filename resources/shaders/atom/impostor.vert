#version 430

layout(location = 0) in vec3 positionAttribute;
layout(location = 1) in vec3 colorAttribute;
layout(location = 2) in float radiusAttribute;

uniform mat4 model;

out vData {
	vec3 color;
	float radius;
	uint index;
} vertex;


void main(){
	gl_Position = model * vec4(positionAttribute, 1.f);
	vertex.color = colorAttribute;
	vertex.radius = radiusAttribute;
	vertex.index = gl_VertexID;
}
