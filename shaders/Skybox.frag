#version 420 core

layout(location = 0) in vec3 TexCoords;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform samplerCube u_CubeTexture;

void main()
{    
    FragColor = texture(u_CubeTexture, TexCoords);
}