
struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
    float4 tColor : COLOR;
    float tTexture : BLENDWEIGHT;
};

Texture2D u_Textures[16] : register(t0);
SamplerState u_Samplers[16] : register(s0);

//we have to do this because an array index has to be a literal
#define SAMPLETEX(x) u_Textures[x].Sample(u_Samplers[x], fsIn.tTexCoords) * fsIn.tColor

float4 main(vsOut fsIn) : SV_TARGET
{
    int index = int(fsIn.tTexture);
    switch(index)
    {
        case 0:
            return SAMPLETEX(0);
        case 1:
            return SAMPLETEX(1);
        case 2:
            return SAMPLETEX(2);
        case 3:
            return SAMPLETEX(3);
        case 4:
            return SAMPLETEX(4);
        case 5:
            return SAMPLETEX(5);
        case 6:
            return SAMPLETEX(6);
        case 7:
            return SAMPLETEX(7);
        case 8:
            return SAMPLETEX(8);
        case 9:
            return SAMPLETEX(9);
        case 10:
            return SAMPLETEX(10);
        case 11:
            return SAMPLETEX(11);
        case 12:
            return SAMPLETEX(12);
        case 13:
            return SAMPLETEX(13);
        case 14:
            return SAMPLETEX(14);
        case 15:
            return SAMPLETEX(15);
    }
    return float4(1.0f,1.0f,1.0f,1.0f);
}