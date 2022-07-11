#version 430

// In / out
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vData {
	flat vec3 capPosition;
	flat vec3 capNormal;
	flat float scale;
} vertex[2];

uniform mat4 view;
uniform mat4 projection;
uniform float scale;

//out
out fData {
    vec2 uv;
	vec4 position;
	flat mat4 cylinderBase;
	flat vec4 start;
	flat vec4 end;
	flat float radius[2];
	flat float helixScale[2];
	flat vec4 capPosition[2];
	flat vec4 capNormal[2];
} fragment;



// Main function
void main()
{
	
	vec3 viewDir = vec3(view[0][2], view[1][2], view[2][2]); // Third row of view matrix (cameraLeft)
	
	//normal direction between A and B
	vec3 zAxis = normalize(vec3(gl_in[1].gl_Position)-vec3(gl_in[0].gl_Position));
	vec3 yAxis = normalize(cross(viewDir,zAxis));
	vec3 xAxis = normalize(cross(yAxis,zAxis));

	fragment.start = gl_in[0].gl_Position;
	fragment.end = gl_in[1].gl_Position;

	
	fragment.helixScale[0] = vertex[0].scale;
	fragment.helixScale[1] = vertex[1].scale;
	fragment.radius[0] = (0.2f+fragment.helixScale[0]*0.2f)*scale;
	fragment.radius[1] = (0.2f+fragment.helixScale[1]*0.2f)*scale;
	
	fragment.cylinderBase = mat4(
		xAxis.x,yAxis.x,zAxis.x,0,
		xAxis.y,yAxis.y,zAxis.y,0,
		xAxis.z,yAxis.z,zAxis.z,0,
		      0,      0,      0,1
	);
	
	fragment.capPosition[0] = fragment.cylinderBase * ( vec4(vertex[0].capPosition, 1) - fragment.start);
	fragment.capPosition[1] = fragment.cylinderBase * ( vec4(vertex[1].capPosition, 1) - fragment.start);
	mat4 tranInvCylinderBase = transpose(inverse(fragment.cylinderBase));
	fragment.capNormal[0] = normalize( tranInvCylinderBase * vec4(vertex[0].capNormal, 0));
	fragment.capNormal[1] = normalize( tranInvCylinderBase * vec4(vertex[1].capNormal, 0));
	
	float rMax = max(fragment.radius[0],fragment.radius[1]);
	// Combine matrices
    mat4 M = projection * view;
	// Emit quad
	fragment.position = (gl_in[0].gl_Position +  rMax*1.2 * vec4(yAxis-xAxis-zAxis*10, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(1,1);
	EmitVertex();

	fragment.position = (gl_in[1].gl_Position +  rMax*1.2 * vec4(yAxis-xAxis+zAxis*10, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(-1,1);
	EmitVertex();

	fragment.position = (gl_in[0].gl_Position +  rMax*1.2 * vec4(-yAxis-xAxis-zAxis*10, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(1,-1);
	EmitVertex();

	fragment.position = (gl_in[1].gl_Position +  rMax*1.2 * vec4(-yAxis-xAxis+zAxis*10, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(-1,-1);
	EmitVertex();
	EndPrimitive();


}