#version 420 core
out vec4 FragColor;
  
in vec2 TexCoords;

layout(binding = 0) uniform sampler2D u_ScreenTexture;

void main()
{ 
    FragColor = texture(u_ScreenTexture, TexCoords);
}