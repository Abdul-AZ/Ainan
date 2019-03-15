#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float BlurStrength[3];

void main()
{ 
    FragColor += texture(screenTexture, vec2(TexCoords.x - 2, TexCoords.y)) * BlurStrength[2];
    FragColor += texture(screenTexture, vec2(TexCoords.x - 1, TexCoords.y)) * BlurStrength[1];
    FragColor += texture(screenTexture, vec2(TexCoords.x	, TexCoords.y)) * BlurStrength[0];
    FragColor += texture(screenTexture, vec2(TexCoords.x + 1, TexCoords.y)) * BlurStrength[1];
    FragColor += texture(screenTexture, vec2(TexCoords.x + 2, TexCoords.y)) * BlurStrength[2];
}