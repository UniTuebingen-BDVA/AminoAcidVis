#version 330

//in
in fData {
    vec2 uv;
	vec4 position;
	flat vec3 color[2];
	flat mat4 cylinderBase;
	flat vec4 start;
	flat vec4 end;
	flat float radius;
	flat int index[2];
} fragment;

uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightDir = vec3(-1,-1,-1);
uniform vec3 cameraWorldPos;

uniform float depthDarkeningStart = 200.f;
uniform float depthDarkeningEnd = 400f;

out vec4 fragColor;
out float gl_FragDepth;
out int picking;

void main() {
	//fragColor = vec4(fragment.uv.x,fragment.uv.y,0,1);
	//return;
	vec3 viewDir = vec3(view[0][2], view[1][2], view[2][2]); // Third row of view matrix (cameraLeft)
	vec4 cameraPos = vec4(view[0][3],view[1][3],view[2][3],1);
	
	// The rays origin and direction gets transformed into the base of the cylinder to make the calculations easier
	vec4 o = fragment.cylinderBase*(fragment.position-fragment.start); //Ray Start
	vec4 v  = fragment.cylinderBase*vec4(viewDir,0); //Ray Direction

	// Insert ray 'P = o + tv' into cylinder '0 = x^2 + y^2 - r^2' => '0 = (o.x + t v.x)^2 + (o.y + t v.y)^2 - r^2'
	// then solve for t => https://www.wolframalpha.com/input/?i=solve+(o1%2Bt*v1)%5E2+%2B+(o2%2Bt*v2)%5E2-r%5E2+%3D+0+for+t
	float radius2 = fragment.radius*fragment.radius;
	float vx2 = v.x*v.x;
	float vy2 = v.y*v.y;
	float inSqrt =  2*o.x*o.y*v.x*v.y - o.x*o.x*vy2 - o.y*o.y*vx2 + radius2*(vx2+vy2);
	if(inSqrt < 0) discard; // no intersection
	
	float a1 = sqrt(inSqrt);
	float a2 = - o.x*v.x - o.y*v.y;
	float a3 = vx2 + vy2; 
	
	float t = -(a1 + a2) / a3;
	vec4 p = o-v*t;
	float height = length(fragment.end-fragment.start); //the height of the cylinder
	if(p.z < 0 || p.z > height) discard; // point bigger then the cylinder height
	// Now transform it back to the world base form the cylinder base
	vec4 worldNormal =  normalize(transpose(fragment.cylinderBase) * vec4(p.x,p.y,0,0));
	vec4 worldPos = ((inverse(fragment.cylinderBase) * p)+fragment.start); //CG3
	

	// Set depth of pixel by projecting pixel position into clip space
	vec4 viewPos = view * worldPos; 
	vec4 projPos = projection * viewPos;
	float projDepth = projPos.z / projPos.w;
	float customDepth = (projDepth + 1.0) * 0.5;  // gl_FragCoord.z is from 0..1. So go from clip space to viewport space
	gl_FragDepth = customDepth;
	
	// Depth darkening
    float depthDarkening = (-viewPos.z - depthDarkeningStart) / (depthDarkeningEnd - depthDarkeningStart);
    depthDarkening = 1;//1.0 - clamp(depthDarkening, 0, 1);
	
	// Diffuse lighting (hacked together, not correct)
	vec4 normal =  view * worldNormal;
    vec4 nrmLightDirection = normalize(vec4(lightDir, 0));
    float lighting = max(0,dot(normal.xyz, (view * -nrmLightDirection).xyz));
	
    // Specular lighting (camera pos in view matrix last column is in view coordinates?)
    vec3 reflectionVector = reflect(nrmLightDirection.xyz, worldNormal.xyz);
    vec3 surfaceToCamera = normalize(cameraWorldPos - worldPos.xyz);
    float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
    float specular = pow(cosAngle, 10);
	
	// Some "ambient" lighting combined with specular
	vec3 color;
	int index;
	if(p.z < height/2){
		color = fragment.color[0];
		index = fragment.index[0];
	}else{
		color = fragment.color[1];
		index = fragment.index[1];
	}
    vec3 finalColor = depthDarkening * mix(color * mix(vec3(0.4, 0.45, 0.5), vec3(1.0, 1.0, 1.0), lighting), vec3(1,1,1), specular);

    // Rim lighting
    finalColor += ((0.75 * lighting) + 0.25) * pow(1.0 - dot(normal.xyz, vec3(0,0,1)), 3);
	
	//fog
	float dis = (length(cameraWorldPos - worldPos.xyz)-190)/40;
	finalColor = mix(finalColor, vec3(0,0,0), clamp(dis,0,0.7f) );
	
	fragColor = vec4(finalColor,1);
	
	picking = index;
}
