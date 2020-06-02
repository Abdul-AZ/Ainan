
struct vsOut
{
    float4 tPos : SV_POSITION;
    float2 tTexCoords : TEXCOORD0;
};

Texture2D u_FrameTexture : register(t0);
SamplerState u_FrameSampler : register(s0);

cbuffer BlurData : register(b1)
{
    float2 u_Resolution;
    float2 u_BlurDirection; //this is {1.0, 0.0} for the horizontal pass and {0.0, 1.0} for vertical pass
    float u_Radius;
};

float4 main(vsOut fsIn) : SV_TARGET
{
    float blurX = u_Radius / u_Resolution.x;
    float blurY = u_Radius / u_Resolution.y;

	float4 FragColor = float4(0.0, 0.0, 0.0, 0.0);

    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x - 4.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y - 4.0 * blurY * u_BlurDirection.y)) * 0.017867;
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x - 3.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y - 3.0 * blurY * u_BlurDirection.y)) * 0.054364;
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x - 2.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y - 2.0 * blurY * u_BlurDirection.y)) * 0.120338;
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x - 1.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y - 1.0 * blurY * u_BlurDirection.y)) * 0.193829;

    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x, fsIn.tTexCoords.y)) * 0.227204;
    
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x + 1.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y + 1.0 * blurY * u_BlurDirection.y)) * 0.193829;
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x + 2.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y + 2.0 * blurY * u_BlurDirection.y)) * 0.120338;
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x + 3.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y + 3.0 * blurY * u_BlurDirection.y)) * 0.054364;
    FragColor += u_FrameTexture.Sample(u_FrameSampler, float2(fsIn.tTexCoords.x + 4.0 * blurX * u_BlurDirection.x,fsIn.tTexCoords.y + 4.0 * blurY * u_BlurDirection.y)) * 0.017867;

    return FragColor;
}