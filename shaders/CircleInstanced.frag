#version 330 core

in vec4 color;
in vec2 TextureCoordinates;
out vec4 FragColor;

uniform sampler2D particleTexture;

void main()
{
    FragColor = texture(particleTexture, TextureCoordinates) * color;
}