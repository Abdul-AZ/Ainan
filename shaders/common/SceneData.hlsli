cbuffer FrameData : register(b0)
{
    matrix<float, 4, 4> u_ViewProjection;

	float2 RadialLightPosition[10];
	float4 RadialLightColor[10];
	float  RadialLightIntensity[10];

	float2 SpotLightPosition[10];
	float4 SpotLightColor[10];
	float  SpotLightAngle[10];
	float  SpotLightInnerCutoff[10];
	float  SpotLightOuterCutoff[10];
	float  SpotLightIntensity[10];
}