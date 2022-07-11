//============================================================================
// Author: Vladimir Ageev
//============================================================================

// To make this work you need to set these settigns:
//	glEnable(GL_PROGRAM_POINT_SIZE); //enables the gl_PointSize member
//	glEnable(GL_POINT_SPRITE); // enables the gl_PointCoord member (in newer open gl this is always enabled)
//	glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT); // sets the uv origin (could be done in fragment)
/// @see https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)

#version 430

layout(location = 0) in vec3 positionAttribute;
layout(location = 1) in vec3 colorAttribute;
layout(location = 2) in float layerAttribute;
layout(location = 3) in float radiusAttribute;
layout(location = 4) in int groupIDAttribute;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float zoom;
uniform float scale = 1.0f;
uniform float propeRadius = 0.0f;

uniform int hoveredAtom = -1;
uniform int hoveredGroup = -1;
uniform int selectedAtom = -1;
uniform int selectedGroup = -1;

uniform bool equalSize = false;

uniform int maxLayer = -1;

uniform int colorMode = 0;

//visibility
uniform bool hideWater = true;
uniform bool onlyBackBone = false;
uniform int waterSkip = 0;
uniform bool applyFilters = true;

layout (std140, binding = 0) uniform colors{
	vec4 layerColors[1];
};


out vData {
	flat vec3 center;
	flat vec3 color;
	flat int layer;
	flat float radius;
	flat bool hide;
} vertex;

vec3 hsv2rgb(vec3 c){
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 getColor(int colorMode, vec3 colorAttribute, float layerAttribute, int groupID, bool water){
	//if(propeRadius > 0) return vec3(1,1,1);
	switch (colorMode) {
		case 1:
			{return colorAttribute;}
		case 2:
			{return (water)?  vec3(0,0,0.9):hsv2rgb(vec3(groupID/6.0,0.9,0.9));}
		case 3:
			{return (layerAttribute < 0)? vec3(1,1,1) : layerColors[int(layerAttribute)].rgb;}
		case 4:{
			int i = int(floor(layerAttribute));
			float rest = layerAttribute-i;
			vec4 colorA = layerColors[i];
			vec4 colorB = layerColors[i+1];
			return mix(colorA,colorB, rest).rgb;
		}
		default:{
			float strenght = (layerAttribute)/float(maxLayer-1);
			return mix(colorAttribute, vec3(1.0,1.0,1.0), strenght*0.7);
		}
	}
}

void main(){
	bool hidden = bool(groupIDAttribute & 0x00010000);
	bool water =  bool(groupIDAttribute & 0x00020000);
	bool backbone =  bool(groupIDAttribute & 0x00040000);
	bool hidden_filter = bool(groupIDAttribute & 0x00080000);
	int groupID = groupIDAttribute & 0x0000FFFF;
	if( (hidden_filter && applyFilters) || hidden || (hideWater && water) || (bool(waterSkip) && water && bool(mod(groupID,waterSkip))) || ( !backbone && !water && onlyBackBone ) ){
		vertex.hide = true;
		return;
	}else{
		vertex.hide = false;
	}

	gl_Position = projection * view * model * vec4(positionAttribute, 1.f);

	vertex.radius = (propeRadius > 0.f)? radiusAttribute+propeRadius : (equalSize)? scale : (radiusAttribute)*scale;
	//if(layerAttribute < 2 ) vertex.radius = 0;
	gl_PointSize = vertex.radius*zoom;
	vertex.center = positionAttribute;
	vertex.color = getColor( (propeRadius > 0.f)?3:colorMode, colorAttribute, layerAttribute, groupID, water);

	if(groupID == hoveredGroup) vertex.color+=vec3(0.4,0.4,0.4);
	if(gl_VertexID == hoveredAtom) vertex.color = vec3(0.19f,0.7f,0.96f)*2.f;
	if(groupID == selectedGroup) vertex.color=vec3(1)-vertex.color;
	if(gl_VertexID == selectedAtom) vertex.color=vec3(2.0f,1.9f,1.5f);
}
