#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

//sigma is 3.0

void main()
{ 
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y- 2)) * 0.1784;
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y- 1)) * 0.210431;
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y 	))  * 0.222338;
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y+ 1)) * 0.210431;
    FragColor += texture(screenTexture, vec2(TexCoords.x , TexCoords.y+ 2)) * 0.1784;
}