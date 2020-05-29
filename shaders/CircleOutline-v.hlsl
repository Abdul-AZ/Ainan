
cbuffer FrameData : register(b0)
{
    matrix<float, 4, 4> u_ViewProjection;
}

cbuffer ObjectTransform : register(b1)
{
    matrix<float, 4, 4> u_Model;
}

struct vsInput
{
    float2 vPos : aPos;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
};

vsOut main( vsInput vsIn )
{
    vsOut output;
    output.tPos = mul(u_ViewProjection, mul(u_Model, float4(vsIn.vPos, 0, 1)));
    return output;    
}