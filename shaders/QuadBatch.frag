#version 420 core

layout(location = 0) in vec2 TextureCoordinates;
layout(location = 1) in vec4 Color;
layout(location = 2) in float Texture;
layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_Textures[16];

void main()
{
    int tex = int(Texture);
    FragColor = vec4(0,0,0,0);

    switch(tex)
    {
        case 0:
            FragColor = texture(u_Textures[0], TextureCoordinates) * Color;
            break;
        case 1:
            FragColor = texture(u_Textures[1], TextureCoordinates) * Color;
            break;
        case 2:
            FragColor = texture(u_Textures[2], TextureCoordinates) * Color;
            break;
        case 3:
            FragColor = texture(u_Textures[3], TextureCoordinates) * Color;
            break;
        case 4:
            FragColor = texture(u_Textures[4], TextureCoordinates) * Color;
            break;
        case 5:
            FragColor = texture(u_Textures[5], TextureCoordinates) * Color;
            break;
        case 6:
            FragColor = texture(u_Textures[6], TextureCoordinates) * Color;
            break;
        case 7:
            FragColor = texture(u_Textures[7], TextureCoordinates) * Color;
            break;
        case 8:
            FragColor = texture(u_Textures[8], TextureCoordinates) * Color;
            break;
        case 9:
            FragColor = texture(u_Textures[9], TextureCoordinates) * Color;
            break;
        case 10:
            FragColor = texture(u_Textures[10], TextureCoordinates) * Color;
            break;
        case 11:
            FragColor = texture(u_Textures[11], TextureCoordinates) * Color;
            break;
        case 12:
            FragColor = texture(u_Textures[12], TextureCoordinates) * Color;
            break;
        case 13:
            FragColor = texture(u_Textures[13], TextureCoordinates) * Color;
            break;
        case 14:
            FragColor = texture(u_Textures[14], TextureCoordinates) * Color;
            break;
        case 15:
            FragColor = texture(u_Textures[15], TextureCoordinates) * Color;
            break;
    }
}