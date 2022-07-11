#version 430

// In / out
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in vData {
	flat vec3 color;
	flat int layer;
	flat float radius;
	flat int index;
	flat bool hide;
} vertex[2];

uniform mat4 view;
uniform mat4 projection;

//out
out fData {
    vec2 uv;
	vec4 position;
	flat vec3 color[2];
	flat mat4 cylinderBase;
	flat vec4 start;
	flat vec4 end;
	flat float radius;
	flat int index[2];
} fragment;


// Main function
void main()
{
	fragment.start = gl_in[0].gl_Position;
	fragment.end = gl_in[1].gl_Position;
	if(vertex[0].hide || vertex[1].hide || distance(fragment.start, fragment.end) > 5 ) return;

	vec3 viewDir = vec3(view[0][2], view[1][2], view[2][2]); // Third row of view matrix (cameraLeft)

	//normal direction between A and B
	vec3 zAxis = normalize(vec3(gl_in[1].gl_Position)-vec3(gl_in[0].gl_Position));
	vec3 yAxis = normalize(cross(viewDir,zAxis));
	vec3 xAxis = normalize(cross(yAxis,zAxis));

	fragment.color[0] = vertex[0].color;
	fragment.color[1] =  vertex[1].color;
	fragment.radius = min(vertex[0].radius,vertex[1].radius);
	fragment.index[0] = vertex[0].index;
	fragment.index[1] = vertex[1].index;

	fragment.cylinderBase = mat4(
		xAxis.x,yAxis.x,zAxis.x,0,
		xAxis.y,yAxis.y,zAxis.y,0,
		xAxis.z,yAxis.z,zAxis.z,0,
		      0,      0,      0,1
	);

	// Combine matrices
    mat4 M = projection * view;
	// Emit quad
	fragment.position = (gl_in[0].gl_Position +  fragment.radius * vec4(yAxis-xAxis-zAxis*26, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(1,1);
	EmitVertex();

	fragment.position = (gl_in[1].gl_Position +  fragment.radius * vec4(yAxis-xAxis+zAxis*26, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(-1,1);
	EmitVertex();

	fragment.position = (gl_in[0].gl_Position +  fragment.radius * vec4(-yAxis-xAxis-zAxis*26, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(1,-1);
	EmitVertex();

	fragment.position = (gl_in[1].gl_Position +  fragment.radius * vec4(-yAxis-xAxis+zAxis*26, 0));
	gl_Position = M * fragment.position;
	fragment.uv = vec2(-1,-1);
	EmitVertex();
	EndPrimitive();
}
