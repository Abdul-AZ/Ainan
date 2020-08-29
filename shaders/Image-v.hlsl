#include <common/SceneData.hlsli>

struct vsInput
{
    float2 vPos : aPos;
    float2 TexCoords : aTexCoords;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
};

vsOut main( vsInput vsIn )
{
    vsOut output;
    output.tPos = float4(vsIn.vPos, 0, 1);
    output.tTexCoords = vsIn.TexCoords;
    return output;    
}