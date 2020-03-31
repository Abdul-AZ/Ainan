#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTextureCoordinates;

uniform mat4 u_ViewProjection;
uniform vec4 u_ColorArr[40];
uniform vec2 u_TranslationArr[40];
uniform float u_ScaleArr[40];

out vec2 TextureCoordinates;
out vec4 Color;

void main()
{
	vec4 pos = vec4(aPos * u_ScaleArr[gl_InstanceID] + u_TranslationArr[gl_InstanceID], 0.0, 1.0);
    gl_Position = u_ViewProjection *  pos;
	Color = u_ColorArr[gl_InstanceID];
	TextureCoordinates = aTextureCoordinates;
}