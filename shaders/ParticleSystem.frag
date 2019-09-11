#version 330 core

in vec4 Color;
in vec2 TextureCoordinates;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main()
{
    FragColor = texture(u_Texture, TextureCoordinates) * Color;
}