#version 450

layout(location = 0) in vec3 positionAttribute;
layout(location = 1) in int idAttribute;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float scale = 1.f;
uniform int colorMode = 0;

uniform int maxLayer = -1;

uniform int hoveredGroup = -1;

uniform int aminoAcidGroupOffset = 0;

layout (std140, binding = 0) uniform colors{
	vec4 layerColors[1];
};

layout (std140, binding = 1) uniform data0{
    struct {
		vec4 pos;
		vec4 capNormal;
		vec4 capOrintation;
	}spline[1];
};

layout (std140, binding = 2) uniform data1{
	vec4 resedueLayer[1];
};




out vData {
	vec3 color;
	vec4 pos;
	vec4 normal;
	flat int resedueID;
} vertex;

float getRadius(float strength){
	return (0.2f+strength*0.4f)*scale;
}

vec3 hsv2rgb(vec3 c){
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

mat4 getRotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


vec3 getColor(int colorMode, vec3 colorAttribute, float layerAttribute, int groupIDAttribute){
	switch (colorMode) {
		case 1:
			{return colorAttribute;}
		case 2:
			{return hsv2rgb(vec3(groupIDAttribute/6.0,0.9,0.9));}
		case 3: {
			if(maxLayer < 0 ) return colorAttribute;
			return (layerAttribute < 0)? vec3(1,1,1) : layerColors[int(layerAttribute)].rgb;
			}
		case 4:{
			if(maxLayer < 0 ) return colorAttribute;
			int i = int(floor(layerAttribute));
			float rest = layerAttribute-i;
			vec4 colorA = layerColors[i];
			vec4 colorB = layerColors[i+1];
			return mix(colorA,colorB, rest).rgb;
		}
		default:{
			return colorAttribute; //mix(colorAttribute, vec3(1.0,1.0,1.0), strenght*0.7);
		}
	}
}

void main(){
	vec4 posAndStengh = spline[idAttribute].pos;
	vec3 xAxis = spline[idAttribute].capNormal.xyz; //xAxis
	vec3 yAxis = spline[idAttribute].capOrintation.xyz;//yAxis
	vec3 zAxis = cross(xAxis,yAxis)*(1.0f - 0.7f*(posAndStengh.a));//zAxis
	//mat4 rotationMatrix = getRotationMatrix( cross(vec3(0,1,0),dir), -acos(dot(vec3(0,1,0),dir))  );

	mat4 basis =  mat4(
		yAxis.x,yAxis.y,yAxis.z,0,
		xAxis.x,xAxis.y,xAxis.z,0,
		zAxis.x,zAxis.y,zAxis.z,0,
		      0,      0,      0,1
	);

	vec4 rotated =  basis*vec4(positionAttribute*getRadius(posAndStengh.a),1);
	vertex.pos = model * ( vec4(posAndStengh.rgb,0) + rotated);
	gl_Position =  projection * view * vertex.pos;
	//gl_Position =  projection * view * model * vec4(positionAttribute.x, idAttribute, positionAttribute.z,1);

	vertex.resedueID = aminoAcidGroupOffset + (idAttribute+6)/6;
	if(hoveredGroup == vertex.resedueID)
		vertex.color = vec3(1.9f,1.9f,2);
	else{
		int id = vertex.resedueID-1;
		vertex.color = getColor(colorMode, mix(vec3(0.25,0.25,0.25), vec3(0.6,0.7,0.8), posAndStengh.a),resedueLayer[id/4][id%4],vertex.resedueID);
	}


	mat4 normalMat = inverse(transpose( model* basis));
	vertex.normal = normalize(normalMat*vec4(positionAttribute,0));
}
