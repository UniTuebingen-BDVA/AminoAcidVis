//============================================================================
// Author: Raphael Menges
//============================================================================

#version 430

// In / out
layout(points) in;

layout(triangle_strip, max_vertices = 4) out;

//in
in vData {
	vec3 color;
	float radius;
	uint index;
} vertex[1];
//out
out fData {
    vec2 uv;
	flat float radius;
	flat vec3 center;
	flat vec3 color;
	flat uint index;
} fragment;


// Uniforms
uniform mat4 view;
uniform mat4 projection;

// Main function
void main()
{
    // Get center
    fragment.center = gl_in[0].gl_Position.xyz;

    // Set color and radius which is for all vertices the same
    fragment.color = vertex[0].color;
    fragment.radius = vertex[0].radius;

    // Index of atom
    fragment.index = vertex[0].index;

    // GLSL is column-major! Get world space camera vectors
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]); // First row of view matrix
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]); // Second row of view matrix

    // Combine matrices
    mat4 M = projection * view;

    // Emit quad
    gl_Position = M * (gl_in[0].gl_Position + vertex[0].radius * vec4(cameraRight+cameraUp, 0));
    fragment.uv = vec2(1,1);
    EmitVertex();

    gl_Position = M * (gl_in[0].gl_Position + vertex[0].radius * vec4(-cameraRight+cameraUp, 0));
    fragment.uv = vec2(-1,1);
    EmitVertex();

    gl_Position = M * (gl_in[0].gl_Position + vertex[0].radius * vec4(cameraRight-cameraUp, 0));
    fragment.uv = vec2(1,-1);
    EmitVertex();

    gl_Position = M * (gl_in[0].gl_Position + vertex[0].radius * vec4(-cameraRight-cameraUp, 0));
    fragment.uv = vec2(-1,-1);
    EmitVertex();

    EndPrimitive();
}
