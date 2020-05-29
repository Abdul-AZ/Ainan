
cbuffer FrameData : register(b0)
{
    matrix<float, 4, 4> u_ViewProjection;
}

struct vsInput
{
    float2 vPos : aPos;
    float4 vColor : aColor;
    float vTexture : aTexture;
    float2 vTexCoords : aTexCoords;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
    float4 tColor : COLOR;
    float tTexture : BLENDWEIGHT;
};

vsOut main( vsInput vsIn )
{
    vsOut output;
    output.tPos = mul(u_ViewProjection, float4(vsIn.vPos, 0.0, 1.0));
	output.tColor = vsIn.vColor;
	output.tTexture = vsIn.vTexture;
	output.tTexCoords = vsIn.vTexCoords;

    return output;    
}