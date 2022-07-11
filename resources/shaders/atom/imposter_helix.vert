#version 330

layout (location = 0) in vec3 positionAttribute;
layout (location = 1) in vec3 capAttribute;
layout (location = 2) in vec3 capNormalAttribute;
layout (location = 3) in float scaleAttribute;
layout (location = 4) in vec3 orintationAttribute;

uniform mat4 model;

out vData {
	flat vec3 capPosition;
	flat vec3 capNormal;
	flat float scale;
} vertex;

void main()
{
	gl_Position = model * vec4(positionAttribute,1);
	vertex.capPosition = capAttribute;
	vertex.capNormal = capNormalAttribute;
	vertex.scale = scaleAttribute;
}