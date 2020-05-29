
struct vsInput
{
    float2 vPos : aPos;
};

struct vsOut
{
    float4 tPos : SV_POSITION;
};

vsOut main(vsInput vsIn)
{
    vsOut vs_out;

    vs_out.tPos = float4(vsIn.vPos.xy, 0.0f, 1.0f);

    return vs_out;
}