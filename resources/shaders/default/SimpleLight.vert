#version 450

layout(location = 0) in vec4 positionAttribute;
layout(location = 1) in vec3 normalAttribute;
layout(location = 2) in vec2 uvAttribute;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 color = vec4(0.1);

out vData {
    vec3 vertex;
	vec3 normal;
	vec2 uv;
	vec4 color;
} vertex;


void main(){
	vec4 position = view * model * vec4(1,0,0,0);//positionAttribute;
	vertex.vertex = vec3(position);
	mat4 my_view = mat4( 0.707107 ,  -0.408248 ,  0.57735 ,  0 ,  0 ,  0.816497 ,  0.57735 ,  0 ,  -0.707107 ,  -0.408248 ,  0.57735 ,  0 ,  0 ,  0 ,  -3.4641 ,  1 );
	mat4 my_proj = mat4( 0.00347826 ,  0 ,  0 ,  0 ,  0 ,  0.00337268 ,  0 ,  0 ,  0 ,  0 ,  -0.153846 ,  0 ,  -1 ,  -1 ,  -0.846154 ,  1 );
	gl_Position =  projection * view * vec4(positionAttribute.xyz, 1.f);
	vertex.normal = ( transpose( inverse( view * model ) ) * vec4(normalAttribute,1) ).xyz;
	vertex.uv = uvAttribute;
	vertex.color = vec4(normalAttribute,1);
}
