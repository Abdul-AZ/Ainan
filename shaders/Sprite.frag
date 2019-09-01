#version 330 core
out vec4 FragColor;
  
in vec2 v_TexCoords;

uniform sampler2D u_SpriteTexture;
uniform vec4 u_Tint;

void main()
{ 
    FragColor = texture(u_SpriteTexture, v_TexCoords) * u_Tint;
}