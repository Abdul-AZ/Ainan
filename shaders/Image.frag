#version 420 core

layout(location = 0) in vec2 TexCoords;

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_ScreenTexture;

void main()
{ 
    FragColor = texture(u_ScreenTexture, TexCoords);
}