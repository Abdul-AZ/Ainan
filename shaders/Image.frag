#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D u_ScreenTexture;

void main()
{ 
    FragColor = texture(u_ScreenTexture, TexCoords);
}