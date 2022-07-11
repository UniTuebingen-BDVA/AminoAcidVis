#version 330

in fData {
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

uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightDir = vec3(-1,-1,-1);
uniform vec3 cameraWorldPos;


out float gl_FragDepth;
out vec4 fragColor;

float pow2(float x){
	return x*x;
}

void main() {

	vec3 viewDir = vec3(view[0][2], view[1][2], view[2][2]); // Third row of view matrix (cameraLeft)
	vec4 cameraPos = vec4(view[0][3],view[1][3],view[2][3],1);

	// The rays origin and direction gets transformed into the base of the cylinder to make the calculations easier
	vec4 o = fragment.cylinderBase*(fragment.position-fragment.start); //Ray Start
	vec4 v  = fragment.cylinderBase*vec4(viewDir,0); //Ray Direction

	float s = (fragment.radius[1]-fragment.radius[0])/ (length(fragment.end-fragment.start));
	float s2 = s*s;
	float r0 = fragment.radius[0];
	float r02 = r0*r0;
	float vx2 = v.x*v.x;
	float vy2 = v.y*v.y;
	float vz2 = v.z*v.z;

	float ox2 = o.x*o.x;
	float oy2 = o.y*o.y;
	float oz2 = o.z*o.z;


	float inSqrt = (oy2+ox2)*s2*vz2
                   +(((-2*o.y*o.z*s2)-2*o.y*r0*s)*v.y
                    +((-2*o.x*o.z*s2)-2*o.x*r0*s)*v.x)*v.z
					+(oz2*s2+2*o.z*r0*s+r02-ox2)*vy2
                   +2*o.x*o.y*v.x*v.y
                   +(oz2*s2+2*o.z*r0*s+r02-oy2)*vx2;

	if(inSqrt < 0) discard; // no intersection

	float f1 = ((-o.z*s2)-r0*s)*v.z+o.y*v.y+o.x*v.x;

	float t = ( sqrt( inSqrt ) - f1)/(s2*vz2-vy2-vx2);
	vec4 p = o-v*t;

	float height = length(fragment.end-fragment.start); //the height of the cylinder
	if(dot(p-fragment.capPosition[0],fragment.capNormal[0]) < 0 || dot(p-fragment.capPosition[1],fragment.capNormal[1]) > 0) discard;

	// Now transform it back to the world base form the cylinder base
	vec4 worldNormal =  normalize(transpose(fragment.cylinderBase) * vec4(p.x,p.y,0,0));
	vec4 worldPos = ((inverse(fragment.cylinderBase) * p)+fragment.start);

	// Set depth of pixel by projecting pixel position into clip space
	vec4 viewPos = view * worldPos;
	vec4 projPos = projection * viewPos;
	float projDepth = projPos.z / projPos.w;
	float customDepth = (projDepth + 1.0) * 0.5;
	gl_FragDepth = customDepth;


	// Diffuse lighting (hacked together, not correct)
	vec4 normal =  view * worldNormal;
    vec4 nrmLightDirection = normalize(vec4(lightDir, 0));
    float lighting = max(0,dot(normal.xyz, (view * -nrmLightDirection).xyz));

    // Specular lighting (camera pos in view matrix last column is in view coordinates?)
    vec3 reflectionVector = reflect(nrmLightDirection.xyz, worldNormal.xyz);
    vec3 surfaceToCamera = normalize(cameraWorldPos - worldPos.xyz);
    float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
    float specular = pow(cosAngle, 10);

	float helixScale =  fragment.helixScale[0] + p.z/height*(fragment.helixScale[1] - fragment.helixScale[0]);
	vec3 finalColor = mix(vec3(0,helixScale,0.2f) * mix(vec3(0.4, 0.45, 0.5), vec3(1.0, 1.0, 1.0), lighting), vec3(1,1,1), specular);

    // Rim lighting
    finalColor += ((0.75 * lighting) + 0.25) * pow(1.0 - dot(normal.xyz, vec3(0,0,1)), 3);

	//fog
	float dis = (length(cameraWorldPos - worldPos.xyz)-190)/40;
	finalColor = mix(finalColor, vec3(0,0,0), clamp(dis,0,0.7f) );

	fragColor = vec4(finalColor,1);
}
