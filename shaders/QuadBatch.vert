#version 420 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aTexture;
layout(location = 3) in vec2 aTexCoords;

#include <common/SceneData.glsli>

layout(location = 0) out vec2 TextureCoordinates;
layout(location = 1) out vec4 Color;
layout(location = 2) out float Texture;

void main()
{
    gl_Position = u_ViewProjection * vec4(aPos, 1.0);
	Color = aColor;
	Texture = aTexture;
	TextureCoordinates = aTexCoords;
}