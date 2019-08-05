#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTextureCoordinates;

uniform mat4 u_ViewProjection;
uniform vec4 u_ColorArr[40];
uniform mat4 u_ModelArr[40];

out vec2 TextureCoordinates;
out vec4 Color;

void main()
{
    gl_Position = u_ViewProjection * u_ModelArr[gl_InstanceID] * vec4(aPos.x, aPos.y, 0.0, 1.0);
	Color = u_ColorArr[gl_InstanceID];
	TextureCoordinates = aTextureCoordinates;
}