#version 420 core

out vec4 FragColor;

in vec2 FragPos;

#define MAX_NUM_RADIAL_LIGHTS 10
#define MAX_NUM_SPOT_LIGHTS 10

#include <common/SceneData.glsli>

layout (std140, binding = 2) uniform LightingData
{
	vec3 u_BaseColor;
	float u_BaseLight;
	float u_Constant;
	float u_Linear;
	float u_Quadratic;
};

void main()
{
	FragColor = vec4(u_BaseColor * u_BaseLight, 1.0);

	for(int i = 0; i < MAX_NUM_RADIAL_LIGHTS; i++) 
	{
		vec3 color = vec3(RadialLightColor[i]);
		
		float distance    = length(RadialLightPosition[i] - FragPos);
		float attenuation = 1.0 / (u_Constant + u_Linear * distance + u_Quadratic * (distance * distance)); 
		
		color *= RadialLightIntensity[i];
		color *= attenuation;
		color *= u_BaseColor;
	
		FragColor += vec4(color.xyz, 1.0);
	}
	
	for(int i = 0; i < MAX_NUM_SPOT_LIGHTS; i++) 
	{
		vec3 color = vec3(SpotLightColor[i]);
	
		float distance = length(SpotLightPosition[i] - FragPos);
		float attenuation = 1.0 / (u_Constant + u_Linear * distance + u_Quadratic * (distance * distance)); 
	
		vec2 lightDir  = normalize(SpotLightPosition[i] - FragPos);
	
		vec2 lightFace = vec2(cos(SpotLightAngle[i]), sin(SpotLightAngle[i]));
	
		float theta = dot(lightDir, normalize(-lightFace));
		float epsilon   = cos(SpotLightInnerCutoff[i]) - cos(SpotLightOuterCutoff[i]);
		float intensity = clamp((theta - cos(SpotLightOuterCutoff[i])) / epsilon, 0.0, 1.0);  
	
		if(theta > cos(SpotLightOuterCutoff[i]))
		   FragColor += vec4(color.xyz, 1.0) * intensity * SpotLightIntensity[i] * attenuation;
	}
}