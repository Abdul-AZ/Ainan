struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tFragPos : VPOS;
};

#define MAX_NUM_RADIAL_LIGHTS 10
#define MAX_NUM_SPOT_LIGHTS 10

#include <common/SceneData.hlsli>

cbuffer LightingData : register(b2)
{
	float3 u_BaseColor;
	float u_BaseLight;
	float u_Constant;
	float u_Linear;
	float u_Quadratic;
};

float4 main(vsOut fsIn) : SV_TARGET
{
    float4 FragColor = float4(u_BaseColor * u_BaseLight, 1.0);
{
	for(int i = 0; i < MAX_NUM_RADIAL_LIGHTS; i++) 
	{
		float3 color = float3(RadialLightColor[i].xyz);
		
		float distance    = length(RadialLightPosition[i] - fsIn.tFragPos);
		float attenuation = 1.0 / (u_Constant + u_Linear * distance + u_Quadratic * (distance * distance)); 
		
		color *= RadialLightIntensity[i];
		color *= attenuation;
		color *= u_BaseColor;

		FragColor += float4(color.xyz, 1.0);
	}
}
{
	for(int i = 0; i < MAX_NUM_SPOT_LIGHTS; i++) 
	{
		float3 color = float3(SpotLightColor[i].xyz);

		float distance = length(SpotLightPosition[i] - fsIn.tFragPos);
		float attenuation = 1.0 / (u_Constant + u_Linear * distance + u_Quadratic * (distance * distance)); 

		float2 lightDir  = normalize(SpotLightPosition[i] - fsIn.tFragPos);

		float2 lightFace = float2(cos(SpotLightAngle[i]), sin(SpotLightAngle[i]));

		float theta = dot(lightDir, normalize(-lightFace));
		float epsilon   = cos(SpotLightInnerCutoff[i]) - cos(SpotLightOuterCutoff[i]);
		float intensity = clamp((theta - cos(SpotLightOuterCutoff[i])) / epsilon, 0.0, 1.0);  

		if(theta > cos(SpotLightOuterCutoff[i]))
		   FragColor += float4(color.xyz, 1.0) * intensity * SpotLightIntensity[i] * attenuation;
	}
}

    return FragColor;
}