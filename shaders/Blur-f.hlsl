
struct vsOut
{
    float4 tPos : SV_POSITION;
};

float4 main(vsOut fsIn) : SV_TARGET
{
    return float4(0,0,0,0);
}