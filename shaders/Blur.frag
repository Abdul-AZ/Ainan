#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D u_BlurTarget;

layout (std140) uniform BlurData
{
    vec2 u_Resolution;
    vec2 u_BlurDirection; //this is {1.0, 0.0} for the horizontal pass and {0.0, 1.0} for vertical pass
    float u_Radius;
};

//this is a 9 pass filter
void main() 
{
    float blurX = u_Radius / u_Resolution.x;
    float blurY = u_Radius / u_Resolution.y;

	FragColor = vec4(0.0);

    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x - 4.0 * blurX * u_BlurDirection.x,TexCoords.y - 4.0 * blurY * u_BlurDirection.y)) * 0.017867;
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x - 3.0 * blurX * u_BlurDirection.x,TexCoords.y - 3.0 * blurY * u_BlurDirection.y)) * 0.054364;
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x - 2.0 * blurX * u_BlurDirection.x,TexCoords.y - 2.0 * blurY * u_BlurDirection.y)) * 0.120338;
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x - 1.0 * blurX * u_BlurDirection.x,TexCoords.y - 1.0 * blurY * u_BlurDirection.y)) * 0.193829;

    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x,TexCoords.y)) * 0.227204;
    
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x + 1.0 * blurX * u_BlurDirection.x,TexCoords.y + 1.0 * blurY * u_BlurDirection.y)) * 0.193829;
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x + 2.0 * blurX * u_BlurDirection.x,TexCoords.y + 2.0 * blurY * u_BlurDirection.y)) * 0.120338;
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x + 3.0 * blurX * u_BlurDirection.x,TexCoords.y + 3.0 * blurY * u_BlurDirection.y)) * 0.054364;
    FragColor += texture2D(u_BlurTarget, vec2(TexCoords.x + 4.0 * blurX * u_BlurDirection.x,TexCoords.y + 4.0 * blurY * u_BlurDirection.y)) * 0.017867;

    FragColor = vec4(FragColor.xyz, 1.0);
}