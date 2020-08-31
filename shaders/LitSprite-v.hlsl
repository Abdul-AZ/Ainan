#include <common/SceneData.hlsli>

struct vsInput
{
    float2 vPos : aPos;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tFragPos : VPOS;
};

cbuffer ObjectData : register(b1)
{
    matrix<float, 4, 4> u_Model;
    float4  u_Tint;
    float u_BaseLight;
    float u_MaterialConstantCoefficient;
    float u_MaterialLinearCoefficient;
    float u_MaterialQuadraticCoefficient;
}

vsOut main(vsInput vsIn)
{
    vsOut vs_out;

    vs_out.tFragPos = mul(u_Model, float4(vsIn.vPos, 0.0f, 1.0f)).xy;
    vs_out.tPos = mul(u_ViewProjection, mul(u_Model, float4(vsIn.vPos, 0.0f, 1.0f)));

    return vs_out;
}