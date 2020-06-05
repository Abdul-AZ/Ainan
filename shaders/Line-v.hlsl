
cbuffer FrameData : register(b0)
{
    matrix<float, 4, 4> u_ViewProjection;
}

struct vsInput
{
    float2 vPos : aPos;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
};

vsOut main( vsInput vsIn )
{
    vsOut output;
    output.tPos = mul(u_ViewProjection, float4(vsIn.vPos, 0, 1));
    return output;    
}