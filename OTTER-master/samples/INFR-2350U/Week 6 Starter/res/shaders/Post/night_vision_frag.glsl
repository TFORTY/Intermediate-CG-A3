#version 420

layout (location = 0) in vec2 inUV;

out vec4 frag_color;

layout (binding = 0) uniform sampler2D s_Tex;

uniform float u_Luminosity;

//https://tobiasbu.wordpress.com/2016/01/16/opengl-night-vision-shader/
void main()
{
	//Load in the source image with its UVs
	vec4 source = texture(s_Tex, inUV);

	//Green colour is dominant. Need to eliminate the red and blue channels but set a color proportion
	//30% red, 59% green, 11% blue = 100% intensity
	//Returned value will be the intensity of the color fragment
	float luminosity = (source.r * 0.3f) + (source.g * 0.59f) + (source.b * 0.11f);

	//Mixes just the green channel 
	frag_color.rgb = mix(source.rgb, vec3(0, luminosity, 0), u_Luminosity);
	frag_color.a = source.a;
}