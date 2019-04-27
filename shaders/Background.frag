#version 330 core

out vec4 FragColor;

uniform vec3 baseColor;
uniform float baseLight;

in vec2 FragPos;

#define MAX_NUM_RADIAL_LIGHTS 10

struct RadialLights
{
	vec2 Position[MAX_NUM_RADIAL_LIGHTS];
	vec3 Color[MAX_NUM_RADIAL_LIGHTS];
	float Constant[MAX_NUM_RADIAL_LIGHTS];
	float Linear[MAX_NUM_RADIAL_LIGHTS];
	float Quadratic[MAX_NUM_RADIAL_LIGHTS];
};

uniform RadialLights radialLights;

void main()
{
	FragColor = baseLight * vec4(baseColor, 1.0);
	for(int i = 0; i < MAX_NUM_RADIAL_LIGHTS; i++) {
		float distance    = length(radialLights.Position[i] - FragPos);
		float attenuation = 1.0 / (radialLights.Constant[i] + radialLights.Linear[i] * distance +  radialLights.Quadratic[i] * (distance * distance)); 
		FragColor += vec4(baseColor, 1.0) * vec4(radialLights.Color[i], 1.0) * attenuation;
	}
}