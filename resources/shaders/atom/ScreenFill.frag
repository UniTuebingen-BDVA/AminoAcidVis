#version 330

in vec2 passTCoord;
flat in mat4 passInvViewProj;

uniform sampler2D diffusetexture;
uniform sampler2D surfacedepthtexture;

uniform bool drawSurface = false;

uniform float zoom;
uniform float glowIntensity = 1.0f;
uniform float blackBorderSize = -1.0f;
uniform bool flipblackBorder = false;

out vec4 fragmentColor;

vec4 getWorldSpacePositionFromDepth(float depth){
        vec4 pos = vec4(passTCoord*2.0 - 1.0,depth* 2.0 - 1.0,1.0);
        pos = passInvViewProj*pos;
        return pos/pos.w;
    }

///https://github.com/rreusser/glsl-solid-wireframe
float gridFactor (vec3 parameter, float width, float feather) {
  float w1 = width - feather * 0.5;
  vec3 d = fwidth(parameter);
  vec3 looped = 0.5 - abs(mod(parameter, 1.0) - 0.5);
  vec3 a3 = smoothstep(d * w1, d * (w1 + feather), looped);
  return min(min(a3.x, a3.y), a3.z);
}
	
void main() {
	fragmentColor = texture(diffusetexture, passTCoord)*glowIntensity;

	if(drawSurface){
		// get screen-space position
		float depth = texture(surfacedepthtexture, passTCoord).r;
		vec4 pos = getWorldSpacePositionFromDepth(depth);
		
		if(depth < 1.00f){
			float grid = gridFactor(vec3(pos), 0.3f, 2.0f);
			float s = 0.62f;
			float d = 0.74f;
			depth = 1-((depth-s)/(d-s));
			//fragmentColor.rgb = fragmentColor.rgb*grid + vec3(0,0.5f,0.9f)*depth*(1.f-grid);
			//fragmentColor.rgb = fragmentColor.rgb*grid + pos.rgb/40.f*depth*(1.f-grid);
			fragmentColor.rgb = fragmentColor.rgb*grid +  vec3(pos.r/40.f,0.5f,0.9f)*depth*(1.f-grid);
			if(grid < 1.f)
				fragmentColor.a = max(1.f-grid,fragmentColor.a);
		}
	}
	if(blackBorderSize > 0.f)
		if(flipblackBorder){
			if(passTCoord.x < blackBorderSize || passTCoord.x > (1.f - blackBorderSize))
				fragmentColor *= 0.4;
		}else{
			if(passTCoord.y < blackBorderSize || passTCoord.y > (1.f - blackBorderSize))
				fragmentColor *= 0.4;
		}
}