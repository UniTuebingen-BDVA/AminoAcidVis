#version 450

in vData {
	vec3 color;
	vec4 pos; //world
	vec4 normal; //world
	flat int resedueID;
} vertex;



uniform mat4 view;

uniform vec3 lightDir = vec3(-1,-1,-1);
uniform vec3 cameraWorldPos;

uniform float depthDarkeningStart = 200.f;
uniform float depthDarkeningEnd = 400f;

out vec4 fragColor;
out int picking;

void main() {
    fragColor = vec4(vertex.normal.rgb,1);
	
	// Diffuse lighting (hacked together, not correct)
	vec4 normal =  view * vertex.normal;
    vec4 nrmLightDirection = normalize(vec4(lightDir, 0));
    float lighting = max(0,dot(normal.xyz, (view * -nrmLightDirection).xyz));
	
    // Specular lighting (camera pos in view matrix last column is in view coordinates?)
    vec3 reflectionVector = reflect(nrmLightDirection.xyz, vertex.normal.xyz);
    vec3 surfaceToCamera = normalize(cameraWorldPos - vertex.pos.xyz);
    float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
    float specular = pow(cosAngle, 10);
	
	vec3 finalColor = mix(vertex.color * mix(vec3(0.4, 0.45, 0.5), vec3(1.0, 1.0, 1.0), lighting), vec3(1,1,1), specular);
	
    // Rim lighting
    finalColor += ((0.2 * lighting) + 0.25) * pow(1.0 - dot(normal.xyz, vec3(0,0,1)), 1);
	
	//fog
	float dis = (length(cameraWorldPos - vertex.pos.xyz)-190)/40;
	finalColor = mix(finalColor, vec3(0,0,0), clamp(dis,0,0.7f) );
	
	fragColor = vec4(finalColor,1.0f);
	picking = vertex.resedueID;
}
