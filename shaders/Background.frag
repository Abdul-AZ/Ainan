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

uniform vec3 u_BaseColor;
uniform float u_BaseLight;

uniform float u_Constant;
uniform float u_Linear;
uniform float u_Quadratic;

uniform RadialLights u_RadialLights;
uniform SpotLights u_SpotLights;

void main()
{
	FragColor = vec4(u_BaseColor * u_BaseLight, 1.0);

	for(int i = 0; i < MAX_NUM_RADIAL_LIGHTS; i++) {
		vec3 color = u_RadialLights.Color[i];
		
		float distance    = length(u_RadialLights.Position[i] - FragPos);
		float attenuation = 1.0 / (u_Constant + u_Linear * distance + u_Quadratic * (distance * distance)); 
		
		color *= u_RadialLights.Intensity[i];
		color *= attenuation;
		color *= u_BaseColor;

		FragColor += vec4(color.xyz, 1.0);
	}

	for(int i = 0; i < MAX_NUM_SPOT_LIGHTS; i++) {
		vec3 color = u_SpotLights.Color[i];

		float distance = length(u_SpotLights.Position[i] - FragPos);
		float attenuation = 1.0 / (u_Constant + u_Linear * distance + u_Quadratic * (distance * distance)); 

		vec2 lightDir  = normalize(u_SpotLights.Position[i] - FragPos);

		vec2 lightFace = vec2(cos(u_SpotLights.Angle[i]), sin(u_SpotLights.Angle[i]));

		float theta = dot(lightDir, normalize(-lightFace));
		float epsilon   = cos(u_SpotLights.InnerCutoff[i]) - cos(u_SpotLights.OuterCutoff[i]);
		float intensity = clamp((theta - cos(u_SpotLights.OuterCutoff[i])) / epsilon, 0.0, 1.0);  

		if(theta > cos(u_SpotLights.OuterCutoff[i]))
		   FragColor += vec4(color.xyz, 1.0) * intensity * u_SpotLights.Intensity[i] * attenuation;
	}
}