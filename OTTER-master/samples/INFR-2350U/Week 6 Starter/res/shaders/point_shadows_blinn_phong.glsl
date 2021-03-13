#version 420

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inFragPosLightSpace;

struct PointLight
{
	vec4 _lightPosition;

	vec4 _lightCol;

	vec4 _ambientCol;

	float _lightLinearFalloff;
	float _lightQuadraticFalloff;
	float _ambientPow;
	float _lightAmbientPow;
	float _lightSpecularPow;

	float _shadowBias;
};

layout (std140, binding = 0) uniform u_Lights
{
	PointLight light;
};

layout (binding = 30) uniform sampler2D s_ShadowMap;
uniform sampler2D s_Diffuse;
uniform sampler2D s_Diffuse2;
uniform sampler2D s_Specular;

uniform float u_TextureMix;
uniform vec3  u_CamPos;

out vec4 frag_color;

float ShadowCalculation(vec4 fragPosLightSpace, float bias)
{
	//Perspective division
	vec3 projectionCoordinates = fragPosLightSpace.xyz / fragPosLightSpace.w;
	
	//Transform into a [0,1] range
	projectionCoordinates = projectionCoordinates * 0.5 + 0.5;
	
	//Get the closest depth value from light's perspective (using our 0-1 range)
	float closestDepth = texture(s_ShadowMap, projectionCoordinates.xy).r;

	//Get the current depth according to the light
	float currentDepth = projectionCoordinates.z;

	//Check whether there's a shadow
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	//Return the value
	return shadow;
}