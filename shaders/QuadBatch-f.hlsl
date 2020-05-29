
struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
    float4 tColor : COLOR;
    float tTexture : BLENDWEIGHT;
};

//Texture2D u_Textures[16] : register(t0);
//SamplerState u_Samplers[16] : register(s0);

float4 main( vsOut fsIn ) : SV_TARGET
{
   // return  u_Textures[int(fsIn.tTexture)].Sample(u_Samplers[int(fsIn.tTexture)], fsIn.tTexCoords) * fsIn.tColor;
   return float4(1.0f,1.0f,1.0f,1.0f);
}