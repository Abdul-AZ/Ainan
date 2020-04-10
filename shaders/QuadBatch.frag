#version 330 core

in vec4 Color;
in vec2 TextureCoordinates;
in float Texture;
out vec4 FragColor;

uniform sampler2D u_Textures[16];

void main()
{
    FragColor = texture(u_Textures[int(Texture)], TextureCoordinates) * Color;
}