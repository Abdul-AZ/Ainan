struct vsOut
{
    float4 tPos : SV_POSITION;
};

cbuffer FrameData : register(b1)
{
    float4 u_Color;
}

float4 main(vsOut fsIn) : SV_TARGET
{
	return u_Color;
}