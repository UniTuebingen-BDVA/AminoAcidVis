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

	fragColor = vec4(fragment.color, 1.f);
}
