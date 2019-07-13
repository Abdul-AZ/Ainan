#version 330 core

out vec4 FragColor;

in vec2 FragPos;

#define MAX_NUM_RADIAL_LIGHTS 10
#define MAX_NUM_SPOT_LIGHTS 10

struct RadialLights
{
	vec2 Position[MAX_NUM_RADIAL_LIGHTS];
	vec3 Color[MAX_NUM_RADIAL_LIGHTS];
	float Intensity[MAX_NUM_RADIAL_LIGHTS];
};

struct SpotLights
{
	vec2 Position[MAX_NUM_SPOT_LIGHTS];
	vec3 Color[MAX_NUM_SPOT_LIGHTS];
	float Angle[MAX_NUM_SPOT_LIGHTS];
	float InnerCutoff[MAX_NUM_SPOT_LIGHTS];
	float OuterCutoff[MAX_NUM_SPOT_LIGHTS];
	float Intensity[MAX_NUM_SPOT_LIGHTS];
};

uniform vec3 baseColor;
uniform float baseLight;

uniform float constant;
uniform float linear;
uniform float quadratic;

uniform RadialLights radialLights;
uniform SpotLights spotLights;

void main()
{
	FragColor = vec4(baseColor * baseLight, 1.0);

	for(int i = 0; i < MAX_NUM_RADIAL_LIGHTS; i++) {
		vec3 color = radialLights.Color[i];
		
		float distance    = length(radialLights.Position[i] - FragPos);
		float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance)); 
		
		color *= radialLights.Intensity[i];
		color *= attenuation;
		color *= baseColor;

		FragColor += vec4(color.xyz, 1.0);
	}

	for(int i = 0; i < MAX_NUM_SPOT_LIGHTS; i++) {
		vec3 color = spotLights.Color[i];

		float distance = length(spotLights.Position[i] - FragPos);
		float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance)); 

		vec2 lightDir  = normalize(spotLights.Position[i] - FragPos);

		vec2 lightFace = vec2(cos(spotLights.Angle[i]), sin(spotLights.Angle[i]));

		float theta = dot(lightDir, normalize(-lightFace));
		float epsilon   = cos(spotLights.InnerCutoff[i]) - cos(spotLights.OuterCutoff[i]);
		float intensity = clamp((theta - cos(spotLights.OuterCutoff[i])) / epsilon, 0.0, 1.0);  

		if(theta > cos(spotLights.OuterCutoff[i]))
		   FragColor += vec4(color.xyz, 1.0) * intensity * spotLights.Intensity[i] * attenuation;
	}
}