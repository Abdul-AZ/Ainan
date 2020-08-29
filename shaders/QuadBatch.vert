#version 420 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aTexture;
layout(location = 3) in vec2 aTexCoords;

#include <common/SceneData.glsli>

out vec2 TextureCoordinates;
out vec4 Color;
out float Texture;

void main()
{
    gl_Position = u_ViewProjection *  vec4(aPos, 0.0, 1.0);
	Color = aColor;
	Texture = aTexture;
	TextureCoordinates = aTexCoords;
}