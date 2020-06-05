Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
};

float4 main(vsOut fsIn) : SV_TARGET
{
	return shaderTexture.Sample(sampleType, fsIn.tTexCoords);
}