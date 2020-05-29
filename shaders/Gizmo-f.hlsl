
struct vsOut
{
    float4 tPos : SV_POSITION;
};

cbuffer ObjectColor : register(b2)
{
    float4 u_Color;
}

float4 main(vsOut fsIn) : SV_TARGET
{
	return u_Color;
}