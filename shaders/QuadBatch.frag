#version 420 core

in vec4 Color;
in vec2 TextureCoordinates;
in float Texture;
out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_Textures[16];

void main()
{
    FragColor = texture(u_Textures[int(Texture)], TextureCoordinates) * Color;
}