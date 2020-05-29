
struct vsOut
{
    float4 tPos : SV_POSITION;
};

float4 main(vsOut fsIn) : SV_TARGET
{
    float4 color = (1.0f,1.0f,1.0f,1.0f);

    return color;
}