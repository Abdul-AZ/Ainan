#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float BlurStrength[3];

void main()
{ 
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y- 2)) * BlurStrength[2];
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y- 1)) * BlurStrength[1];
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y 	))  * BlurStrength[0];
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y+ 1)) * BlurStrength[1];
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y+ 2)) * BlurStrength[2];
}