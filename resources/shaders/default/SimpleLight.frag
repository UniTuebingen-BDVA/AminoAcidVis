#version 450

out vec4 fragColor;

uniform float textureStrength = 0.5;
//uniform sampler2D tex;

uniform float ambientLight = 0.8;
uniform float specStrengh = 0.5;
uniform float shininess = 6.0;
uniform vec3 specColor = vec3(1,1,1);

uniform vec3 lightSrc = vec3(10000,10000,10000);

in vData {
    vec3 vertex;
	vec3 normal;
	vec2 uv;
	vec4 color;
} fragment;

void main() {
    vec3 N = fragment.normal;
    vec3 pos = fragment.vertex.xyz;

    vec3 lightDir = normalize(vec3(lightSrc - pos));

    float lambertian = max(dot(lightDir,N), 0.0);

    vec3 reflectDir = reflect(-lightDir, fragment.normal);
    vec3 viewDir = normalize(-fragment.vertex.xyz);

    float specAngle = max(dot(reflectDir, viewDir), 0.0);
    float specular = pow(specAngle, shininess);

    vec3 color = fragment.color.rgb*(1.f-textureStrength);// + texture(tex, fragment.uv).rgb*textureStrength;

    fragColor = fragment.color;
	
	//vec4( ambientLight*color+lambertian*color*(1-ambientLight) +
                        //specular*specColor*max(lambertian,ambientLight)*specStrengh, 1.0);
}
