
struct vsInput
{
    float2 vPos : aPos;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tFragPos : VPOS;
};

cbuffer FrameData : register(b0)
{
    matrix<float, 4, 4> u_ViewProjection;
};

cbuffer ObjectTransform : register(b1)
{
    matrix<float, 4, 4> u_Model;
}

vsOut main(vsInput vsIn)
{
    vsOut vs_out;

    vs_out.tFragPos = mul(u_Model, float4(vsIn.vPos, 0.0f, 1.0f)).xy;
    vs_out.tPos = mul(u_ViewProjection, mul(u_Model, float4(vsIn.vPos, 0.0f, 1.0f)));

    return vs_out;
}