#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTextureCoordinates;

uniform vec4 colorArr[40];
uniform mat4 model[40];
uniform mat4 u_ViewProjection;

out vec2 TextureCoordinates;
out vec4 color;

void main()
{
    gl_Position = u_ViewProjection * model[gl_InstanceID] * vec4(aPos.x, aPos.y, 0.0, 1.0);
	color = colorArr[gl_InstanceID];
	TextureCoordinates = aTextureCoordinates;
}