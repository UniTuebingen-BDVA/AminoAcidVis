//============================================================================
// Author: Raphael Menges
//============================================================================

#version 430

// In / out
in fData {
    vec2 uv;
	flat float radius;
	flat vec3 center;
	flat vec3 color;
	flat uint index;
} fragment;

out vec4 fragColor;
out float gl_FragDepth; // No information about less because may not be discared
                        // before checked whether atom is part of analysis group
                        // and whether fragment inserted in image for rendering
                        // group atoms on top of molecule.
// Uniforms
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightDir = vec3(-1,-1,-1);
uniform vec3 cameraWorldPos;

uniform float depthDarkeningStart = 200.f;
uniform float depthDarkeningEnd = 400f;

// Main function
void main()
{
    // Radius in UV space is 1 (therefore the scaling with 2 in geometry shader)

    // Distance from center of sphere
    float distance = length(fragment.uv);
    if(distance > 1.0)
    {
            discard;
    }

    // Calculate normal of sphere
    float z = sqrt(1.0 - dot(fragment.uv,fragment.uv)); // 1.0 -((uv.x*uv.x) + (uv.y*uv.y)));
    vec3 normal = normalize(vec3(fragment.uv, z));

	// World space position on sphere
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]); // First row of view matrix
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]); // Second row of view matrix
    vec3 cameraDepth = vec3(view[0][2], view[1][2], view[2][2]); // Third row of view matrix
    vec3 relativeViewPos = normal * fragment.radius;
    vec3 relativeWorldPos = vec3(relativeViewPos.x * cameraRight + relativeViewPos.y * cameraUp + relativeViewPos.z * cameraDepth);
    vec3 worldNormal = normalize(relativeWorldPos);
    vec3 worldPos = fragment.center + relativeWorldPos;

    // Cut at given clipping plane (could be optimized to use less ifs...)
    vec4 viewPos = view * vec4(worldPos, 1);

	 // Set depth of pixel by projecting pixel position into clip space
    float customDepth = 0.0;
	vec4 projPos = projection * view * vec4(worldPos, 1.0);
	float projDepth = projPos.z / projPos.w;
	customDepth = (projDepth + 1.0) * 0.5; // gl_FragCoord.z is from 0..1. So go from clip space to viewport space
    gl_FragDepth = customDepth;

	// Depth darkening
    float depthDarkening = (-viewPos.z - depthDarkeningStart) / (depthDarkeningEnd - depthDarkeningStart);
    depthDarkening = 1.0 - clamp(depthDarkening, 0, 1);

    // Diffuse lighting (hacked together, not correct)
    vec4 nrmLightDirection = normalize(vec4(lightDir, 0));
    float lighting = max(0,dot(normal, (view * -nrmLightDirection).xyz)); // Do it in view space (therefore is normal here ok)


	//vec3 cameraWorldPos = vec3(view[0][3], view[1][3], view[2][3]);
    // Specular lighting (camera pos in view matrix last column is in view coordinates?)
    vec3 reflectionVector = reflect(nrmLightDirection.xyz, worldNormal);
    vec3 surfaceToCamera = normalize(cameraWorldPos - worldPos);
    float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
    float specular = pow(cosAngle, 10);

	// Some "ambient" lighting combined with specular
    vec3 finalColor = depthDarkening * mix(fragment.color.rgb * mix(vec3(0.4, 0.45, 0.5), vec3(1.0, 1.0, 1.0), lighting), vec3(1,1,1), specular);

    // Rim lighting
    finalColor += ((0.75 * lighting) + 0.25) * pow(1.0 - dot(normal, vec3(0,0,1)), 3);

	fragColor = vec4(finalColor, 1.f);

}
